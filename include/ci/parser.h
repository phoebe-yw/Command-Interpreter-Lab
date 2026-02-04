/* CS429 CI Lab
 */

#ifndef CI_PARSER_H
#define CI_PARSER_H
#include "../hashtable/hashtable.h"
#include "command.h"
#include "lexer.h"
#include "token.h"

/**
 * @brief Represents a parser for processing tokens and generating commands.
 *
 * The `Parser` structure is responsible for consuming tokens from a `Lexer`,
 * and maintaining state during parsing.
 */
typedef struct {
    Lexer* lexer;   /**< Pointer to the lexer providing tokens. */
    bool had_error; /**< Flag indicating if an error occurred during parsing. */
    Token current;  /**< The current token being processed. */
    Token next;     /**< The next token to be processed. */

    HashTable* labels; /**< Hash table for storing label positions. */
} Parser;

/**
 * @brief Initializes a `Parser` structure.
 *
 * @param parser Pointer to the `Parser` structure to initialize.
 * @param lexer Pointer to the `Lexer` to be used for tokenizing input.
 * @param labels Pointer to the `HashTable` to be used for storing label
 * positions.
 */
void parser_init(Parser* parser, Lexer* lexer, HashTable* labels);

/**
 * @brief Parses commands from the input token stream.
 *
 * Reads tokens from the associated `Lexer` and builds a linked list of
 * commands. If an error occurs, sets `parser->had_error` to `true`.
 *
 * @param parser Pointer to the initialized `Parser` structure.
 * @return Pointer to the head of a linked list of parsed `Command` objects.
 *         Returns NULL if no commands were parsed or an error occurred.
 *
 * @note The caller is responsible for freeing the memory associated with the
 * returned commands.
 */
Command* parse_commands(Parser* parser);

#endif
