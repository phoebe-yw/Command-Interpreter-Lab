/* CS429 CI Lab
 *
 * Students: Do Not Modify
 */

#ifndef CI_TOKEN_TYPE_H
#define CI_TOKEN_TYPE_H

/**
 * A listing of valid token types that could be outputted by the lexer.
 */
typedef enum {
    TOK_ADD,        /**< add */
    TOK_AND,        /**< and */
    TOK_ASR,        /**< asr */
    TOK_BRANCH,     /**< b (unconditional branch) */
    TOK_BRANCH_EQ,  /**< b.eq */
    TOK_BRANCH_GE,  /**< b.ge */
    TOK_BRANCH_GT,  /**< b.gt */
    TOK_BRANCH_LE,  /**< b.le */
    TOK_BRANCH_LT,  /**< b.lt */
    TOK_BRANCH_NEQ, /**< b.ne */
    TOK_CALL,       /**< call */
    TOK_CMP,        /**< cmp */
    TOK_CMP_U,      /**< cmp_u */
    TOK_COLON,      /**< : */
    TOK_COMMA,      /**< , */
    TOK_EOF,        /**< End of file. */
    TOK_EOR,        /**< eor */
    TOK_ERR,        /**< Error token. */
    TOK_IDENT,      /**< Identifier (e.g. register/base specifier). */
    TOK_LBRACKET,   /**< [ */
    TOK_LOAD,       /**< load */
    TOK_LSL,        /**< lsl */
    TOK_LSR,        /**< lsr */
    TOK_MOV,        /**< mov */
    TOK_NL,         /**< Newline or statement terminator. */
    TOK_NUM,        /**< Number literal (binary/decimal/hex). */
    TOK_ORR,        /**< orr */
    TOK_PRINT,      /**< print */
    TOK_PUT,        /**< put */
    TOK_RBRACKET,   /**< ] */
    TOK_RET,        /**< ret */
    TOK_STORE,      /**< store */
    TOK_STR,        /**< String literal. */
    TOK_SUB,        /**< sub */
    TOK_NOP,        /**< nop */
} TokenType;

#endif
