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

// advance helper method to move on to next token
static void advance(Parser* parser) {
    parser->current = parser->next;
    parser->next = lexer_next_token(parser->lexer);
}

// helper method to set had error to true and free command on error
static Command* fail_cmd(Parser* parser, Command* cmd) {
    parser->had_error = true;  // free after
    free(cmd);
    return NULL;
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
        // week 2 start
        case TOK_NOP: {
            cmd->type = CMD_NOP;
            advance(parser);
            // if the type is not new line AND is not EOF, we have problem
            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                return fail_cmd(parser, cmd);
            }
            return cmd;
        }
        case TOK_MOV: {  // assign a value
            cmd->type = CMD_MOV;
            advance(parser);                          // ident; comma
            if (parser->current.type != TOK_IDENT) {  // expected ident
                return fail_cmd(parser, cmd);
            }
            cmd->destination.type = OP_VAR;
            int64_t var_index;
            if (!parse_ident(parser->current, &var_index)) {
                return fail_cmd(parser, cmd);
            }
            cmd->destination.as.var = var_index;
            advance(parser);  // comma; num
            if (parser->current.type != TOK_COMMA) {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // num; eof
            if (parser->current.type != TOK_NUM) {
                return fail_cmd(parser, cmd);
            }
            cmd->val_a.type = OP_IMM;  // value is an immediate number
            if (!parse_number(parser->current, &cmd->val_a.as.imm)) {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // num; nl
            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                return fail_cmd(parser, cmd);
            }
            return cmd;
        }
        case TOK_PRINT: {
            cmd->type = CMD_PRINT;
            advance(parser);  // ident/num ; comma
            if (parser->current.type == TOK_IDENT) {
                int64_t var_index;
                if (!parse_ident(parser->current, &var_index)) {
                    return fail_cmd(parser, cmd);
                }
                cmd->val_a.type = OP_VAR;
                cmd->val_a.as.var = var_index;
            } else if (parser->current.type == TOK_NUM) {
                int64_t val;
                if (!parse_number(parser->current, &val)) {
                    return fail_cmd(parser, cmd);
                }
                cmd->val_a.type = OP_IMM;
                cmd->val_a.as.imm = val;
            } else {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // comma; base
            if (parser->current.type != TOK_COMMA) {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // base
            if (parser->current.type != TOK_IDENT ||
                parser->current.length != 1) {
                return fail_cmd(parser, cmd);
            }
            char base_type = parser->current.lexeme[0];
            if (base_type != 'B' && base_type != 'X' && base_type != 'D') {
                return fail_cmd(parser, cmd);
            }
            cmd->val_b.type = OP_BASE;
            cmd->val_b.as.base = base_type;
            advance(parser);  // nl; eof
            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                return fail_cmd(parser, cmd);
            }
            return cmd;
        }
        case TOK_ADD:    // fall through to sub
        case TOK_SUB: {  // exact same as add
            cmd->type = (token.type == TOK_ADD) ? CMD_ADD : CMD_SUB;
            advance(parser);  // ident;comma
            if (parser->current.type != TOK_IDENT) {
                return fail_cmd(parser, cmd);
            }
            // set the destination for command
            cmd->destination.type = OP_VAR;
            int64_t var_index;
            if (!parse_ident(parser->current, &var_index)) {
                return fail_cmd(parser, cmd);
            }
            cmd->destination.as.var = var_index;
            advance(parser);  // comma; ident
            if (parser->current.type != TOK_COMMA) {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // ident ; comma
            if (parser->current.type != TOK_IDENT) {
                return fail_cmd(parser, cmd);
            }
            // set val_a in command
            cmd->val_a.type = OP_VAR;
            int64_t val_a_index;
            if (!parse_ident(parser->current, &val_a_index)) {
                return fail_cmd(parser, cmd);
            }
            cmd->val_a.as.var = val_a_index;
            advance(parser);  // comma; ident/num
            if (parser->current.type != TOK_COMMA) {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // ident/num; eof
            // check if it is ident/num
            if (parser->current.type != TOK_IDENT &&
                parser->current.type != TOK_NUM) {
                return fail_cmd(parser, cmd);
            }
            int64_t val_b_index;
            if (parser->current.type == TOK_IDENT) {
                cmd->val_b.type = OP_VAR;
                if (!parse_ident(parser->current, &val_b_index)) {
                    return fail_cmd(parser, cmd);
                }
                cmd->val_b.as.var = val_b_index;
            } else if (parser->current.type == TOK_NUM) {
                cmd->val_b.type = OP_IMM;
                if (!parse_number(parser->current, &val_b_index)) {
                    return fail_cmd(parser, cmd);
                }
                cmd->val_b.as.imm = val_b_index;
            } else {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // eof/nl
            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                return fail_cmd(parser, cmd);
            }
            return cmd;
        }
        case TOK_CMP: {
            cmd->type = CMD_CMP;
            advance(parser);  // ident; comma
            if (parser->current.type != TOK_IDENT) {
                return fail_cmd(parser, cmd);
            }
            // set the val_a for command
            cmd->val_a.type = OP_VAR;
            int64_t val_a_index;
            if (!parse_ident(parser->current, &val_a_index)) {
                return fail_cmd(parser, cmd);
            }
            cmd->val_a.as.var = val_a_index;
            advance(parser);  // comma; ident/num
            if (parser->current.type != TOK_COMMA) {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // ident/num; eof
            if (parser->current.type != TOK_IDENT &&
                parser->current.type != TOK_NUM) {
                return fail_cmd(parser, cmd);
            }
            // set the val_b for command
            int64_t val_b_index;
            if (parser->current.type == TOK_IDENT) {
                cmd->val_b.type = OP_VAR;
                if (!parse_ident(parser->current, &val_b_index)) {
                    return fail_cmd(parser, cmd);
                }
                cmd->val_b.as.var = val_b_index;
            } else if (parser->current.type == TOK_NUM) {
                cmd->val_b.type = OP_IMM;
                if (!parse_number(parser->current, &val_b_index)) {
                    return fail_cmd(parser, cmd);
                }
                cmd->val_b.as.imm = val_b_index;
            }
            advance(parser);  // eof
            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                return fail_cmd(parser, cmd);
            }
            return cmd;
        }
        case TOK_CMP_U: {
            cmd->type = CMD_CMP_U;
            advance(parser);  // ident; comma
            if (parser->current.type != TOK_IDENT) {
                return fail_cmd(parser, cmd);
            }
            // set the val_a for command
            cmd->val_a.type = OP_VAR;
            int64_t val_a_index;
            if (!parse_ident(parser->current, &val_a_index)) {
                return fail_cmd(parser, cmd);
            }
            cmd->val_a.as.var = val_a_index;
            advance(parser);  // comma; ident/num
            if (parser->current.type != TOK_COMMA) {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // ident/num; eof
            if (parser->current.type != TOK_IDENT &&
                parser->current.type != TOK_NUM) {
                return fail_cmd(parser, cmd);
            }
            // set the val_b for command
            int64_t val_b_index;
            if (parser->current.type == TOK_IDENT) {
                cmd->val_b.type = OP_VAR;
                if (!parse_ident(parser->current, &val_b_index)) {
                    return fail_cmd(parser, cmd);
                }
                cmd->val_b.as.var = val_b_index;
            } else if (parser->current.type == TOK_NUM) {
                cmd->val_b.type = OP_IMM;
                if (!parse_number(parser->current, &val_b_index)) {
                    return fail_cmd(parser, cmd);
                }
                cmd->val_b.as.imm = val_b_index;
            }
            advance(parser);  // eof

            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                return fail_cmd(parser, cmd);
            }
            return cmd;
        }
        // week 2 end
        // week 3 start
        case TOK_AND:
        case TOK_ORR:
        case TOK_EOR: {
            if (token.type == TOK_AND) {
                cmd->type = CMD_AND;
            } else if (token.type == TOK_ORR) {
                cmd->type = CMD_ORR;
            } else {
                cmd->type = CMD_EOR;
            }
            advance(parser);  // ident; comma
            if (parser->current.type != TOK_IDENT) {
                return fail_cmd(parser, cmd);
            }
            // set the destination for command
            cmd->destination.type = OP_VAR;
            int64_t var_index;
            if (!parse_ident(parser->current, &var_index)) {
                return fail_cmd(parser, cmd);
            }
            cmd->destination.as.var = var_index;
            advance(parser);  // comma; ident
            if (parser->current.type != TOK_COMMA) {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // ident; comma
            if (parser->current.type != TOK_IDENT) {
                return fail_cmd(parser, cmd);
            }
            // set val_a in command
            cmd->val_a.type = OP_VAR;
            int64_t val_a_index;
            if (!parse_ident(parser->current, &val_a_index)) {
                return fail_cmd(parser, cmd);
            }
            cmd->val_a.as.var = val_a_index;
            advance(parser);  // comma ; ident
            if (parser->current.type != TOK_COMMA) {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // ident; eof
            // check if it is ident
            if (parser->current.type != TOK_IDENT) {
                return fail_cmd(parser, cmd);
            }
            int64_t val_b_index;
            cmd->val_b.type = OP_VAR;
            if (!parse_ident(parser->current, &val_b_index)) {
                return fail_cmd(parser, cmd);
            }
            cmd->val_b.as.var = val_b_index;
            advance(parser);  // eof/nl
            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                return fail_cmd(parser, cmd);
            }
            return cmd;
        }
        case TOK_ASR:
        case TOK_LSR:
        case TOK_LSL: {
            if (token.type == TOK_ASR) {
                cmd->type = CMD_ASR;
            } else if (token.type == TOK_LSR) {
                cmd->type = CMD_LSR;
            } else {
                cmd->type = CMD_LSL;
            }

            advance(parser);  // ident
            if (parser->current.type != TOK_IDENT) {
                return fail_cmd(parser, cmd);
            }
            // set the destination for command
            cmd->destination.type = OP_VAR;
            int64_t var_index;
            if (!parse_ident(parser->current, &var_index)) {
                return fail_cmd(parser, cmd);
            }
            cmd->destination.as.var = var_index;
            advance(parser);  // comma; ident
            if (parser->current.type != TOK_COMMA) {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // ident; comma
            if (parser->current.type != TOK_IDENT) {
                return fail_cmd(parser, cmd);
            }
            // set val_a in command
            cmd->val_a.type = OP_VAR;
            int64_t val_a_index;
            if (!parse_ident(parser->current, &val_a_index)) {
                return fail_cmd(parser, cmd);
            }
            cmd->val_a.as.var = val_a_index;
            advance(parser);  // comma; num
            if (parser->current.type != TOK_COMMA) {
                return fail_cmd(parser, cmd);
            }
            advance(parser);  // num; eof
            // check if it is num
            if (parser->current.type != TOK_NUM) {
                return fail_cmd(parser, cmd);
            }
            int64_t val_b_index;
            cmd->val_b.type = OP_IMM;
            if (!parse_number(parser->current, &val_b_index)) {
                return fail_cmd(parser, cmd);
            }
            cmd->val_b.as.imm = val_b_index;
            advance(parser);  // eof/nl
            if (parser->current.type != TOK_NL &&
                parser->current.type != TOK_EOF) {
                return fail_cmd(parser, cmd);
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
            advance(parser);
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
