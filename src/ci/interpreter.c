/* CS429 CI Lab
 */

#include "interpreter.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
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
    intr->stack = NULL;
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
            case CMD_ADD: {
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
            }
            case CMD_SUB: {
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
                intr->is_greater = false;
                intr->is_less = false;
                intr->is_equal = false;
                int64_t left = intr->variables[current->val_a.as.var];
                int64_t right = (current->val_b.type == OP_VAR)
                                    ? intr->variables[current->val_b.as.var]
                                    : current->val_b.as.imm;

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
                intr->is_greater = false;
                intr->is_less = false;
                intr->is_equal = false;
                uint64_t left =
                    (uint64_t)intr->variables[current->val_a.as.var];
                uint64_t right =
                    (current->val_b.type == OP_VAR)
                        ? (uint64_t)intr->variables[current->val_b.as.var]
                        : (uint64_t)current->val_b.as.imm;

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
                int64_t value = (current->val_a.type == OP_VAR)
                                    ? intr->variables[current->val_a.as.var]
                                    : current->val_a.as.imm;

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
                int64_t res = intr->variables[current->val_a.as.var] &
                              intr->variables[current->val_b.as.var];
                intr->variables[current->destination.as.var] = res;
                break;
            }
            case CMD_ORR: {
                int64_t res = intr->variables[current->val_a.as.var] |
                              intr->variables[current->val_b.as.var];
                intr->variables[current->destination.as.var] = res;
                break;
            }
            case CMD_EOR: {
                int64_t res = intr->variables[current->val_a.as.var] ^
                              intr->variables[current->val_b.as.var];
                intr->variables[current->destination.as.var] = res;
                break;
            }
            case CMD_ASR: {
                int64_t res = intr->variables[current->val_a.as.var] >>
                              current->val_b.as.imm;
                intr->variables[current->destination.as.var] = res;
                break;
            }
            case CMD_LSR: {
                uint64_t res =
                    (uint64_t)intr->variables[current->val_a.as.var] >>
                    (uint64_t)current->val_b.as.imm;
                intr->variables[current->destination.as.var] = res;
                break;
            }
            case CMD_LSL: {
                uint64_t res = (uint64_t)intr->variables[current->val_a.as.var]
                               << (uint64_t)current->val_b.as.imm;
                intr->variables[current->destination.as.var] = res;
                break;
            }
            case CMD_PUT: {
                int64_t addr = (current->val_b.type == OP_IMM)
                                   ? current->val_b.as.imm
                                   : intr->variables[current->val_b.as.var];
                size_t stored_addr = (size_t)addr;
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
            case CMD_STORE: {
                size_t bytes = (size_t)current->val_b.as.imm;
                int64_t offset = (current->val_a.type == OP_VAR)
                                     ? intr->variables[current->val_a.as.var]
                                     : current->val_a.as.imm;
                int64_t source =
                    (int64_t)intr->variables[current->destination.as.var];
                if (!mem_store((uint8_t*)&source, (size_t)offset, bytes)) {
                    intr->had_error = true;
                    return;
                }
                break;
            }
            case CMD_LOAD: {
                size_t bytes = (size_t)current->val_b.as.imm;
                int64_t offset = (current->val_a.type == OP_VAR)
                                     ? intr->variables[current->val_a.as.var]
                                     : current->val_a.as.imm;
                uint64_t value = 0;
                if (!mem_load((uint8_t*)&value, (size_t)offset, bytes)) {
                    intr->had_error = true;
                    return;
                }
                intr->variables[current->destination.as.var] = (int64_t)value;
                break;
            }
            // week 3 end
            // week 4 start
            case CMD_BRANCH: {
                bool branches = false;
                if (current->branch_condition == BRANCH_ALWAYS) {
                    branches = true;
                } else if (current->branch_condition == BRANCH_EQUAL) {
                    branches = intr->is_equal;
                } else if (current->branch_condition == BRANCH_NOT_EQUAL) {
                    branches = !intr->is_equal;
                } else if (current->branch_condition == BRANCH_GREATER) {
                    branches = intr->is_greater;
                } else if (current->branch_condition == BRANCH_LESS) {
                    branches = intr->is_less;
                } else if (current->branch_condition == BRANCH_GREATER_EQUAL) {
                    branches = intr->is_greater || intr->is_equal;
                } else if (current->branch_condition == BRANCH_LESS_EQUAL) {
                    branches = intr->is_less || intr->is_equal;
                } else {
                    intr->had_error = true;
                    return;
                }
                if (branches) {
                    char* label = current->val_a.as.str;
                    Command* target_cmd = (Command*)ht_get(intr->labels, label);
                    if (!target_cmd) {
                        printf("Label not found: %s\n", label);
                        intr->had_error = true;
                        return;
                    }
                    current = target_cmd;
                    continue;
                }
                break;
            }
            case CMD_CALL: {
                StackRecord* new_record = malloc(sizeof(StackRecord));
                if (!new_record) {
                    intr->had_error = true;
                    return;
                }
                // save current variables to stack record
                for (int i = 0; i < NUM_VARIABLES; i++) {
                    new_record->variables[i] = intr->variables[i];
                }
                new_record->return_address = current->next;
                new_record->next = intr->stack;
                intr->stack = new_record;
                // jump to label
                char* label = current->val_a.as.str;
                Command* target_cmd = (Command*)ht_get(intr->labels, label);
                if (!target_cmd) {
                    printf("Label not found: %s\n", label);
                    free(new_record);
                    intr->had_error = true;
                    return;
                }
                current = target_cmd;
                continue;
            }
            case CMD_RET: {
                if (!intr->stack) {
                    current = NULL;
                    return;
                }
                StackRecord* top = intr->stack;
                intr->stack = top->next;
                for (int i = 1; i < NUM_VARIABLES; i++) {
                    intr->variables[i] = top->variables[i];
                }
                Command* return_cmd = top->return_address;
                free(top);
                current = return_cmd;
                continue;
            }
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
