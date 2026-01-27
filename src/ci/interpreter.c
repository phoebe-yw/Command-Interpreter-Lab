/* CS429 CI Lab
 */

#include "interpreter.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "mem.h"

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

// helper to print out base 2 (recursive)
void print_binary(uint64_t value) {
    if (value < 2) {
        if (value == 1) {
            putchar('1');
        } else {
            putchar('0');
        }
        return;
    }
    print_binary(value / 2);
    if (value % 2 == 1) {
        putchar('1');
    } else {
        putchar('0');
    }
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
            // week 2 start
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
                if (current->destination.type != OP_VAR ||
                    current->val_a.type != OP_VAR ||
                    (current->val_b.type != OP_VAR &&
                     current->val_b.type != OP_IMM)) {
                    intr->had_error = true;
                    return;
                }
                int64_t sum;
                if (current->val_b.type == OP_VAR) {
                    sum = intr->variables[current->val_a.as.var] +
                          intr->variables[current->val_b.as.var];
                } else {
                    sum = intr->variables[current->val_a.as.var] +
                          current->val_b.as.imm;
                }
                intr->variables[current->destination.as.var] = sum;
                break;
            case CMD_SUB: {
                if (current->destination.type != OP_VAR ||
                    current->val_a.type != OP_VAR ||
                    (current->val_b.type != OP_VAR &&
                     current->val_b.type != OP_IMM)) {
                    intr->had_error = true;
                    return;
                }
                int64_t diff;
                if (current->val_b.type == OP_VAR) {
                    diff = intr->variables[current->val_a.as.var] -
                           intr->variables[current->val_b.as.var];
                } else {
                    diff = intr->variables[current->val_a.as.var] -
                           current->val_b.as.imm;
                }
                intr->variables[current->destination.as.var] = diff;
                break;
            }
            case CMD_CMP: {
                if (current->val_a.type != OP_VAR ||
                    (current->val_b.type != OP_VAR &&
                     current->val_b.type != OP_IMM)) {
                    intr->had_error = true;
                    return;
                }
                intr->is_greater = false;
                intr->is_less = false;
                intr->is_equal = false;
                int64_t left = intr->variables[current->val_a.as.var];
                int64_t right;

                if (current->val_b.type == OP_VAR) {
                    right = intr->variables[current->val_b.as.var];
                } else if (current->val_b.type == OP_IMM) {
                    right = current->val_b.as.imm;
                }

                if (left > right) {
                    intr->is_greater = true;
                } else if (left < right) {
                    intr->is_less = true;
                } else {
                    intr->is_equal = true;
                }
                break;
            }
            case CMD_CMP_U: {
                if (current->val_a.type != OP_VAR ||
                    (current->val_b.type != OP_VAR &&
                     current->val_b.type != OP_IMM)) {
                    intr->had_error = true;
                    return;
                }
                intr->is_greater = false;
                intr->is_less = false;
                intr->is_equal = false;
                uint64_t left =
                    (uint64_t)intr->variables[current->val_a.as.var];
                uint64_t right;

                if (current->val_b.type == OP_VAR) {
                    right = (uint64_t)intr->variables[current->val_b.as.var];
                } else if (current->val_b.type == OP_IMM) {
                    right = (uint64_t)current->val_b.as.imm;
                }

                if (left > right) {
                    intr->is_greater = true;
                } else if (left < right) {
                    intr->is_less = true;
                } else {
                    intr->is_equal = true;
                }
                break;
            }
            case CMD_PRINT: {
                if (current->val_a.type != OP_VAR &&
                    current->val_a.type != OP_IMM) {
                    intr->had_error = true;
                    return;
                }
                if (current->val_b.type != OP_BASE) {
                    intr->had_error = true;
                    return;
                }
                int64_t value;
                if (current->val_a.type == OP_VAR) {
                    value = intr->variables[current->val_a.as.var];
                } else if (current->val_a.type == OP_IMM) {
                    value = current->val_a.as.imm;
                }

                // now print the correct base B X D
                char base_type = current->val_b.as.base;
                if (base_type == 'D') {  // print decimal
                    printf("%ld\n", value);
                } else if (base_type == 'X') {  // print hex
                    printf("0x%lx\n", (unsigned long)value);
                } else if (base_type == 'B') {  // print binary
                    printf("0b");
                    print_binary(value);
                    printf("\n");
                    // added for week 3
                } else if (base_type == 'S') {  // print string
                    uint64_t addr = (uint64_t)value;
                    if (addr >= MEM_CAPACITY) {
                        intr->had_error = true;
                        return;
                    }
                    for (size_t off = addr; off < MEM_CAPACITY; off++) {
                        uint8_t str;
                        if (!mem_load(&str, off, 1)) {
                            intr->had_error = true;
                            return;
                        }
                        if (str == '\0') {
                            break;
                        }
                        putchar((char)str);
                    }
                    putchar('\n');
                } else {
                    intr->had_error = true;
                    return;
                }
                break;
            }
            // week 2 end
            // week 3 start
            case CMD_AND: {
                if (current->destination.type != OP_VAR ||
                    current->val_a.type != OP_VAR ||
                    current->val_b.type != OP_VAR) {
                    intr->had_error = true;
                    return;
                }
                int64_t res = intr->variables[current->val_a.as.var] &
                              intr->variables[current->val_b.as.var];
                intr->variables[current->destination.as.var] = res;
                break;
            }
            case CMD_ORR: {
                if (current->destination.type != OP_VAR ||
                    current->val_a.type != OP_VAR ||
                    current->val_b.type != OP_VAR) {
                    intr->had_error = true;
                    return;
                }
                int64_t res = intr->variables[current->val_a.as.var] |
                              intr->variables[current->val_b.as.var];
                intr->variables[current->destination.as.var] = res;
                break;
            }
            case CMD_EOR: {
                if (current->destination.type != OP_VAR ||
                    current->val_a.type != OP_VAR ||
                    current->val_b.type != OP_VAR) {
                    intr->had_error = true;
                    return;
                }
                int64_t res = intr->variables[current->val_a.as.var] ^
                              intr->variables[current->val_b.as.var];
                intr->variables[current->destination.as.var] = res;
                break;
            }
            case CMD_ASR: {
                if (current->destination.type != OP_VAR ||
                    current->val_a.type != OP_VAR ||
                    current->val_b.type != OP_IMM) {
                    intr->had_error = true;
                    return;
                }
                int64_t res = intr->variables[current->val_a.as.var] >>
                              current->val_b.as.imm;
                intr->variables[current->destination.as.var] = res;
                break;
            }
            case CMD_LSR: {
                if (current->destination.type != OP_VAR ||
                    current->val_a.type != OP_VAR ||
                    current->val_b.type != OP_IMM) {
                    intr->had_error = true;
                    return;
                }
                uint64_t res =
                    (uint64_t)intr->variables[current->val_a.as.var] >>
                    (uint64_t)current->val_b.as.imm;
                intr->variables[current->destination.as.var] = res;
                break;
            }
            case CMD_LSL: {
                if (current->destination.type != OP_VAR ||
                    current->val_a.type != OP_VAR ||
                    current->val_b.type != OP_IMM) {
                    intr->had_error = true;
                    return;
                }
                uint64_t res = (uint64_t)intr->variables[current->val_a.as.var]
                               << (uint64_t)current->val_b.as.imm;
                intr->variables[current->destination.as.var] = res;
                break;
            }
            case CMD_PUT: {
                if (current->val_a.type != OP_STR ||
                    (current->val_b.type != OP_IMM &&
                     current->val_b.type != OP_VAR)) {
                    intr->had_error = true;
                    return;
                }
                int64_t addr64 = (current->val_b.type == OP_IMM)
                                     ? current->val_b.as.imm
                                     : intr->variables[current->val_b.as.var];
                if (addr64 < 0 || addr64 >= MEM_CAPACITY) {
                    intr->had_error = true;
                    return;
                }
                size_t stored_addr = (size_t)addr64;
                const uint8_t* s = (const uint8_t*)current->val_a.as.str;
                size_t len = strlen(current->val_a.as.str) + 1;

                for (size_t i = 0; i < len; i++) {
                    uint8_t b = s[i];
                    if (!mem_store(&b, stored_addr + i, 1)) {
                        intr->had_error = true;
                        return;
                    }
                }
                break;
            }
            default:
                return;
        }
        current = current->next;
    }
    free_command(commands);  // successful commands freed
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
