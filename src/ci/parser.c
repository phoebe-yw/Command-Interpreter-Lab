/* CS429 CI Lab
 */

#include "parser.h"

#include <stdlib.h>

#include "command_type.h"
#include "token_type.h"

static bool parse_number(Token token, int64_t* result);
static Command* parse_cmd(Parser* parser);

// we have the parser reading from the lexer
void parser_init(Parser* parser, Lexer* lexer) {
    if (!parser) {
        return;
    }

    parser->lexer = lexer;  // set the lexer
    parser->had_error = false;
    parser->current = lexer_next_token(parser->lexer);  // first token
    parser->next = lexer_next_token(parser->lexer);     // second token
}

/**
 * @brief Parses the given value as a number. (checks if token is a number and
 * converts to int64_t)
 *
 * @param token The token to parse.
 * @param result A pointer to the value to modify on success.
 * @return True if `result` was successfully modified, false otherwise.
 */
static bool parse_number(Token token, int64_t* result) {
    const char* parse_start = token.lexeme;

    int base = 0;

    // modified: deal with binary prefix 0b
    if (token.length > 2 && token.lexeme[0] == '0' && token.lexeme[1] == 'b') {
        base = 2;
        parse_start += 2;
    }

    char* endptr;
    *result = strtoll(parse_start, &endptr, base);

    return (token.lexeme + token.length) == endptr;
}

// helper function to parse identifier tokens in the form of x<number>
static bool parse_ident(Token token, int64_t* result) {
    // pass in Token, and variable int64_t we need to modify the mem address of
    if (token.type != TOK_IDENT) {
        return false;
    }

    if (token.lexeme[0] != 'x' || token.length <= 1) {
        return false;
    }

    // reject leading zeros
    if (token.length > 2 && token.lexeme[1] == '0') {
        return false;
    }

    int64_t val;
    Token after_x;
    after_x.type = TOK_IDENT;
    after_x.length = token.length - 1;
    after_x.lexeme = token.lexeme + 1;

    if (!parse_number(after_x, &val)) {
        return false;
    }
    if (val < 0 || val > 31) {
        return false;
    }
    *result = val;
    return true;
}

/**
 * @brief Parses a singular command.
 *
 * Reads in the token(s) from the lexer that the parser owns and determines the
 * appropriate matching command. Updates the parser->had_error if an error
 * occurs.
 *
 * @param parser A pointer to the parser to read tokens from.
 * @return A pointer to the appropriate command.
 * Returns null if an error occurred or there are no commands to parse.
 *
 * @note The caller is responsible for freeing the memory associated with the
 * returned command.
 */
static Command* parse_cmd(Parser* parser) {
    if (!parser) {
        return NULL;
    }

    Token token = parser->current;

    if (token.type == TOK_EOF) {
        // We are at the end and there are no commands left to parse.
        return NULL;
    }

    // TODO: Switch over token types and parse appropriately.
    //
    // See `include/ci/command_type.h` for command types.
    // See `include/ci/token_type.h` for token types.
    //
    // Token types correspond to commands, e.g., TOK_NOP -> CMD_NOP.
    Command* cmd = calloc(1, sizeof(Command));
    if (!cmd) {
        parser->had_error = true;
        return NULL;
    }
    switch (token.type) {
        case TOK_NOP: {
            cmd->type = CMD_NOP;

            parser->current = parser->next;
            parser->next = lexer_next_token(parser->lexer);

            // if the type is not new line AND is not EOF, we have problem
            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                parser->had_error = true;  // free after
                free(cmd);
                return NULL;
            }
            return cmd;
        }
        case TOK_MOV: {  // assign a value
            cmd->type = CMD_MOV;

            parser->current = parser->next;                  // should be ident
            parser->next = lexer_next_token(parser->lexer);  // should be comma

            if (parser->current.type != TOK_IDENT) {  // expected ident
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            cmd->destination.type = OP_VAR;
            int64_t var_index;
            if (!parse_ident(parser->current, &var_index)) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            cmd->destination.as.var = var_index;

            parser->current = parser->next;                  // should be comma
            parser->next = lexer_next_token(parser->lexer);  // should be num

            if (parser->current.type != TOK_COMMA) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            parser->current = parser->next;                  // should be num
            parser->next = lexer_next_token(parser->lexer);  // should be eof
            if (parser->current.type != TOK_NUM) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            cmd->val_a.type = OP_IMM;  // value is an immediate number
            if (!parse_number(parser->current, &cmd->val_a.as.imm)) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            parser->current = parser->next;                  // should be num
            parser->next = lexer_next_token(parser->lexer);  // should be NL
            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            return cmd;
        }
        case TOK_PRINT: {
            cmd->type = CMD_PRINT;

            parser->current = parser->next;  // should be ident or num
            parser->next = lexer_next_token(parser->lexer);  // should be comma

            if (parser->current.type == TOK_IDENT) {
                int64_t var_index;
                if (!parse_ident(parser->current, &var_index)) {
                    parser->had_error = true;
                    free(cmd);
                    return NULL;
                }
                cmd->val_a.type = OP_VAR;
                cmd->val_a.as.var = var_index;
            } else if (parser->current.type == TOK_NUM) {
                int64_t val;
                if (!parse_number(parser->current, &val)) {
                    parser->had_error = true;
                    free(cmd);
                    return NULL;
                }
                cmd->val_a.type = OP_IMM;
                cmd->val_a.as.imm = val;
            } else {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            parser->current = parser->next;                  // should be comma
            parser->next = lexer_next_token(parser->lexer);  // should be base

            if (parser->current.type != TOK_COMMA) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            parser->current = parser->next;  // should be base
            parser->next = lexer_next_token(parser->lexer);

            if (parser->current.type != TOK_IDENT ||
                parser->current.length != 1) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            char base_type = parser->current.lexeme[0];
            if (base_type != 'B' && base_type != 'X' && base_type != 'D') {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            cmd->val_b.type = OP_BASE;
            cmd->val_b.as.base = base_type;

            parser->current = parser->next;  // NL / EOF
            parser->next = lexer_next_token(parser->lexer);

            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            return cmd;
        }
        case TOK_ADD: {
            cmd->type = CMD_ADD;

            parser->current = parser->next;                  // tok ident
            parser->next = lexer_next_token(parser->lexer);  // tok comma

            if (parser->current.type != TOK_IDENT) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            // set the destination for command
            cmd->destination.type = OP_VAR;
            int64_t var_index;
            if (!parse_ident(parser->current, &var_index)) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            cmd->destination.as.var = var_index;

            parser->current = parser->next;                  // tok comma
            parser->next = lexer_next_token(parser->lexer);  // tok ident

            if (parser->current.type != TOK_COMMA) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            parser->current = parser->next;                  // tok ident
            parser->next = lexer_next_token(parser->lexer);  // tok comma

            if (parser->current.type != TOK_IDENT) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            // set val_a in command
            cmd->val_a.type = OP_VAR;
            int64_t val_a_index;
            if (!parse_ident(parser->current, &val_a_index)) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            cmd->val_a.as.var = val_a_index;

            parser->current = parser->next;                  // tok comma
            parser->next = lexer_next_token(parser->lexer);  // tok ident/num

            if (parser->current.type != TOK_COMMA) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            parser->current = parser->next;                  // tok ident/num
            parser->next = lexer_next_token(parser->lexer);  // tok eof

            // check if it is ident/num
            if (parser->current.type != TOK_IDENT &&
                parser->current.type != TOK_NUM) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            int64_t val_b_index;
            if (parser->current.type == TOK_IDENT) {
                cmd->val_b.type = OP_VAR;
                if (!parse_ident(parser->current, &val_b_index)) {
                    parser->had_error = true;
                    free(cmd);
                    return NULL;
                }
                cmd->val_b.as.var = val_b_index;
            } else if (parser->current.type == TOK_NUM) {
                cmd->val_b.type = OP_IMM;
                if (!parse_number(parser->current, &val_b_index)) {
                    parser->had_error = true;
                    free(cmd);
                    return NULL;
                }
                cmd->val_b.as.imm = val_b_index;
            } else {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            parser->current = parser->next;  // tok eof/nl
            parser->next = lexer_next_token(parser->lexer);

            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            return cmd;
        }
        case TOK_SUB: {  // exact same as add
            cmd->type = CMD_SUB;

            parser->current = parser->next;                  // tok ident
            parser->next = lexer_next_token(parser->lexer);  // tok comma

            if (parser->current.type != TOK_IDENT) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            // set the destination for command
            cmd->destination.type = OP_VAR;
            int64_t var_index;
            if (!parse_ident(parser->current, &var_index)) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            cmd->destination.as.var = var_index;

            parser->current = parser->next;                  // tok comma
            parser->next = lexer_next_token(parser->lexer);  // tok ident

            if (parser->current.type != TOK_COMMA) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            parser->current = parser->next;                  // tok ident
            parser->next = lexer_next_token(parser->lexer);  // tok comma

            if (parser->current.type != TOK_IDENT) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            // set val_a in command
            cmd->val_a.type = OP_VAR;
            int64_t val_a_index;
            if (!parse_ident(parser->current, &val_a_index)) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            cmd->val_a.as.var = val_a_index;

            parser->current = parser->next;                  // tok comma
            parser->next = lexer_next_token(parser->lexer);  // tok ident/num

            if (parser->current.type != TOK_COMMA) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            parser->current = parser->next;                  // tok ident/num
            parser->next = lexer_next_token(parser->lexer);  // tok eof

            // check if it is ident/num
            if (parser->current.type != TOK_IDENT &&
                parser->current.type != TOK_NUM) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            int64_t val_b_index;
            if (parser->current.type == TOK_IDENT) {
                cmd->val_b.type = OP_VAR;
                if (!parse_ident(parser->current, &val_b_index)) {
                    parser->had_error = true;
                    free(cmd);
                    return NULL;
                }
                cmd->val_b.as.var = val_b_index;
            } else if (parser->current.type == TOK_NUM) {
                cmd->val_b.type = OP_IMM;
                if (!parse_number(parser->current, &val_b_index)) {
                    parser->had_error = true;
                    free(cmd);
                    return NULL;
                }
                cmd->val_b.as.imm = val_b_index;
            } else {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            parser->current = parser->next;  // tok eof/nl
            parser->next = lexer_next_token(parser->lexer);

            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            return cmd;
        }
        case TOK_CMP: {
            cmd->type = CMD_CMP;

            parser->current = parser->next;                  // ident
            parser->next = lexer_next_token(parser->lexer);  // comma

            if (parser->current.type != TOK_IDENT) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            // set the val_a for command
            cmd->val_a.type = OP_VAR;
            int64_t val_a_index;
            if (!parse_ident(parser->current, &val_a_index)) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            cmd->val_a.as.var = val_a_index;

            parser->current = parser->next;                  // tok comma
            parser->next = lexer_next_token(parser->lexer);  // tok ident/num

            if (parser->current.type != TOK_COMMA) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            parser->current = parser->next;                  // tok ident/num
            parser->next = lexer_next_token(parser->lexer);  // tok eof

            if (parser->current.type != TOK_IDENT &&
                parser->current.type != TOK_NUM) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            // set the val_b for command
            int64_t val_b_index;
            if (parser->current.type == TOK_IDENT) {
                cmd->val_b.type = OP_VAR;
                if (!parse_ident(parser->current, &val_b_index)) {
                    parser->had_error = true;
                    free(cmd);
                    return NULL;
                }
                cmd->val_b.as.var = val_b_index;
            } else if (parser->current.type == TOK_NUM) {
                cmd->val_b.type = OP_IMM;
                if (!parse_number(parser->current, &val_b_index)) {
                    parser->had_error = true;
                    free(cmd);
                    return NULL;
                }
                cmd->val_b.as.imm = val_b_index;
            }

            parser->current = parser->next;  // eof
            parser->next = lexer_next_token(parser->lexer);

            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            return cmd;
        }
        case TOK_CMP_U: {
            cmd->type = CMD_CMP_U;

            parser->current = parser->next;                  // ident
            parser->next = lexer_next_token(parser->lexer);  // comma

            if (parser->current.type != TOK_IDENT) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            // set the val_a for command
            cmd->val_a.type = OP_VAR;
            int64_t val_a_index;
            if (!parse_ident(parser->current, &val_a_index)) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            cmd->val_a.as.var = val_a_index;

            parser->current = parser->next;                  // tok comma
            parser->next = lexer_next_token(parser->lexer);  // tok ident/num

            if (parser->current.type != TOK_COMMA) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            parser->current = parser->next;                  // tok ident/num
            parser->next = lexer_next_token(parser->lexer);  // tok eof

            if (parser->current.type != TOK_IDENT &&
                parser->current.type != TOK_NUM) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }

            // set the val_b for command
            int64_t val_b_index;
            if (parser->current.type == TOK_IDENT) {
                cmd->val_b.type = OP_VAR;
                if (!parse_ident(parser->current, &val_b_index)) {
                    parser->had_error = true;
                    free(cmd);
                    return NULL;
                }
                cmd->val_b.as.var = val_b_index;
            } else if (parser->current.type == TOK_NUM) {
                cmd->val_b.type = OP_IMM;
                if (!parse_number(parser->current, &val_b_index)) {
                    parser->had_error = true;
                    free(cmd);
                    return NULL;
                }
                cmd->val_b.as.imm = val_b_index;
            }

            parser->current = parser->next;  // eof
            parser->next = lexer_next_token(parser->lexer);

            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                parser->had_error = true;
                free(cmd);
                return NULL;
            }
            return cmd;
        }
        default:
            // unrecognized command
            parser->had_error = true;
            free(cmd);
            break;
    }

    return NULL;
}

Command* parse_commands(Parser* parser) {
    // TODO: Build a linked list of commands by repeatedly calling parse_cmd.
    //
    // For now, we try and parse a single command.
    if (!parser) {
        return NULL;
    }
    Command* head = NULL;
    Command* tail = NULL;  // keep track of last command added
    while (!parser->had_error) {
        if (parser->current.type == TOK_EOF) {
            break;
        }
        // skip newlines
        while (parser->current.type == TOK_NL) {
            parser->current = parser->next;
            parser->next = lexer_next_token(parser->lexer);
        }
        Command* new_cmd = parse_cmd(parser);
        if (!new_cmd) {
            break;
        }
        if (head == NULL) {
            head = new_cmd;
            tail = new_cmd;
        } else {
            tail->next = new_cmd;
            tail = new_cmd;
        }
    }
    return head;
}
