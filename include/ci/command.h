/* CS429 CI Lab
 *
 * Students: Do Not Modify
 */

#ifndef CI_COMMAND_H
#define CI_COMMAND_H
#include "command_type.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Enum representing different branching conditions for commands.
 */
typedef enum {
  BRANCH_NONE,
  BRANCH_ALWAYS,
  BRANCH_EQUAL,
  BRANCH_NOT_EQUAL,
  BRANCH_GREATER,
  BRANCH_LESS,
  BRANCH_GREATER_EQUAL,
  BRANCH_LESS_EQUAL,
} BranchCondition;

/**
 * @brief Enum describing the kind of an operand.
 */
typedef enum {
  OP_NONE, /**< Unset/uninitialized operand. */
  OP_VAR,  /**< Variable index (xN). */
  OP_IMM,  /**< Immediate number. */
  OP_STR,  /**< String/label. */
  OP_BASE  /**< Print base: 'D'|'B'|'X'|'S'. */
} OperandType;

/**
 * @brief Tagged union representing an operand and its payload.
 */
typedef struct {
  OperandType type;
  union {
    int64_t var; /**< for OP_VAR: variable index (0..31). */
    int64_t imm; /**< for OP_IMM: immediate value. */
    char *str;   /**< for OP_STR: string or label. */
    char base;   /**< for OP_BASE: base specifier. */
  } as;
} Operand;

/**
 * @brief Represents a command with operands, branching conditions, and
 * metadata.
 */
typedef struct cmd {
  CommandType type; /**< The type of the command. */
  struct cmd *next; /**< Pointer to the next command in the sequence. */
  /**
   * @brief Destination variable that this command writes to.
   *
   * Should always be an integer operand; string variant currently unused.
   */
  Operand destination;
  Operand val_a; /**< The first operand. */
  Operand val_b; /**< The second operand. */
  BranchCondition
      branch_condition; /**< The branching condition for the command. */
} Command;

/**
 * @brief Frees memory associated with a command.
 *
 * @param command Pointer to the `Command` to be freed.
 */
void free_command(Command *command);

/**
 * @brief Prints the details of a command.
 *
 * Outputs information about the command, including its type, operands,
 * destination, and branching condition, in a human-readable format.
 *
 * @param cmd Pointer to the `Command` to print.
 */
void print_command(Command *cmd);

/**
 * @brief Prints the details of a single operand.
 *
 * Outputs the operand's type and value in a human-readable format.
 *
 * @param op The operand to print.
 */
void print_command_op(const Operand *op);

/**
 * @brief Prints a list of commands.
 *
 * Outputs all commands in the given list, one by one, in a human-readable
 * format.
 *
 * @param cmd Pointer to the first `Command` in the list.
 */
void print_commands(Command *cmd);

#endif
