/* CS429 CI Lab
 */

#include "parser.h"

#include <stdlib.h>

#include "command_type.h"
#include "token_type.h"

static bool parse_number(Token token, int64_t* result);
static Command* parse_cmd(Parser* parser);

void parser_init(Parser* parser, Lexer* lexer) {
    if (!parser) {
        return;
    }

    parser->lexer = lexer;
    parser->had_error = false;
    parser->current = lexer_next_token(parser->lexer);
    parser->next = lexer_next_token(parser->lexer);
}

/**
 * @brief Parses the given value as a number.
 *
 * @param token The token to parse.
 * @param result A pointer to the value to modify on success.
 * @return True if `result` was successfully modified, false otherwise.
 */
static bool parse_number(Token token, int64_t* result) {
    const char* parse_start = token.lexeme;

    char* endptr;
    *result = strtoll(parse_start, &endptr, 0);

    return (token.lexeme + token.length) == endptr;
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
    Token token = parser->current;

    if (token.type == TOK_EOF) {
        // We are at the end and there are no commands left to parse.
    }

    // TODO: Switch over token types and parse appropriately.
    //
    // See `include/ci/command_type.h` for command types.
    // See `include/ci/token_type.h` for token types.
    //
    // Token types correspond to commands, e.g., TOK_NOP -> CMD_NOP.
    switch (token.type) {
        default:
            parser->had_error = true;
            break;
    }

    return NULL;
}

Command* parse_commands(Parser* parser) {
    // TODO: Build a linked list of commands by repeatedly calling parse_cmd.
    //
    // For now, we try and parse a single command.
    return parse_cmd(parser);
}
