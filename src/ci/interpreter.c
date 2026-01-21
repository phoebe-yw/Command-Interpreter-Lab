/* CS429 CI Lab
 */

#include "interpreter.h"

#include <inttypes.h>
#include <stdio.h>

void interpreter_init(Interpreter* intr) {
    if (!intr) {
        return;
    }
    // Initialize interpreter state
    // Set all variables to 0, reset flags, etc.
    for (int i = 0; i < NUM_VARIABLES; i++) {
        intr->variables[i] = 0;
    }
    intr->had_error = false;
    intr->is_greater = false;
    intr->is_less = false;
    intr->is_equal = false;
}

void interpret(Interpreter* intr, Command* commands) {
    if (!intr || !commands) {
        return;
    }

    // TODO: Implement the interpreter logic here.
    // Loop through commands, execute them and update interpreter state.
    // Should look a lot like the logic in `parser.c`, except now with command
    // types as opposed to token types.
    Command* current = commands;
    while (current != NULL) {
        switch (current->type) {
            case CMD_NOP:
                break;
            case CMD_MOV: {
                // first check if it is an OP_IMM, then set to val
                if (current->val_a.type != OP_IMM) {
                    intr->had_error = true;
                    return;
                }
                int64_t val = current->val_a.as.imm;
                intr->variables[current->destination.as.var] = val;
                break;
            }
            case CMD_ADD:
                break;
            case CMD_SUB:
                break;
            case CMD_CMP:
                break;
            case CMD_CMP_U:
                break;
            case CMD_PRINT:
                break;
            default:
                return;
        }
        current = current->next;
    }
}

void print_interpreter_state(Interpreter* intr) {
    if (!intr) {
        return;
    }

    printf("Error: %d\n", intr->had_error);
    printf("Flags:\n");
    printf("Is greater: %d\n", intr->is_greater);
    printf("Is equal: %d\n", intr->is_equal);
    printf("Is less: %d\n", intr->is_less);

    printf("\n");

    printf("Variable values:\n");
    for (size_t i = 0; i < NUM_VARIABLES; i++) {
        printf("x%zu: %" PRId64 "", i, intr->variables[i]);

        if (i < NUM_VARIABLES - 1) {
            printf(", ");
        }

        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }

    printf("\n");
}
