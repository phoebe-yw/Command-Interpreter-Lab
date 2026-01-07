/* CS429 CI Lab
 */

#ifndef CI_INTERPRETER_H
#define CI_INTERPRETER_H
#include "command.h"
#include "hashtable/hashtable.h"

/** Maximum number of defined variables. */
#define NUM_VARIABLES 32

/**
 * @brief Represents the state of the interpreter during execution.
 */
typedef struct {
  int64_t variables[NUM_VARIABLES]; /**< Array of variables used in the
                                       interpreter. */
  bool had_error;  /**< Flag indicating if an error occurred during
                      interpretation. */
  bool is_greater; /**< Flag indicating the last comparison result (greater). */
  bool is_less;    /**< Flag indicating the last comparison result (less). */
  bool is_equal;   /**< Flag indicating the last comparison result (equal). */
} Interpreter;

/**
 * @brief Initializes the interpreter state.
 *
 * @param intr Pointer to the `Interpreter` to initialize.
 */
void interpreter_init(Interpreter *intr);

/**
 * @brief Executes a list of commands using the interpreter.
 *
 * @param intr Pointer to the `Interpreter` that will execute the commands.
 * @param commands Pointer to the first `Command` in the list of commands to
 * interpret.
 */
void interpret(Interpreter *intr, Command *commands);

/**
 * @brief Prints the current state of the interpreter.
 *
 * Outputs the interpreter's variables, flags, stack state, and other relevant
 * information in a human-readable format.
 *
 * @param intr Pointer to the `Interpreter` whose state is to be printed.
 */
void print_interpreter_state(Interpreter *intr);

#endif
