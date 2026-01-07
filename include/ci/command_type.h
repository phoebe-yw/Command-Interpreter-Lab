/* CS429 CI Lab
 *
 * Students: Do Not Modify
 */

#ifndef CI_COMMAND_TYPE_H
#define CI_COMMAND_TYPE_H

/**
 * @brief An enum representing the types of commands that can be executed by the
 * interpreter.
 */
typedef enum {
    /** add xD, xA, xB or add xD, xA, imm */
    CMD_ADD,

    /** and xD, xA, xB */
    CMD_AND,

    /** asr xD, xA, shift */
    CMD_ASR,

    /** b[.cond] label (see TokenType for valid condition tokens) */
    CMD_BRANCH,

    /** call label */
    CMD_CALL,

    /** cmp xA, xB or cmp xA, imm */
    CMD_CMP,

    /** Unsigned variant of CMD_CMP. */
    CMD_CMP_U,

    /** Parse error / unknown command. */
    CMD_ERR,

    /** eor xD, xA, xB */
    CMD_EOR,

    /** load xD, [base, offset] */
    CMD_LOAD,

    /** lsl xD, xA, shift */
    CMD_LSL,

    /** lsr xD, xA, shift */
    CMD_LSR,

    /** mov xD, imm */
    CMD_MOV,

    /** orr xD, xA, xB */
    CMD_ORR,

    /** print value, base (base is D/X/B/S) */
    CMD_PRINT,

    /** put "string", address */
    CMD_PUT,

    /** ret */
    CMD_RET,

    /** store xS, [base, offset] */
    CMD_STORE,

    /** sub xD, xA, xB or sub xD, xA, imm */
    CMD_SUB,

    /** No-op command (does nothing). */
    CMD_NOP,
} CommandType;

#endif
