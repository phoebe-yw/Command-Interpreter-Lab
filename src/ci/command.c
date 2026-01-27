/* CS429 CI Lab
 */

#include "command.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

void free_command(Command* command) {
    // TODO: free the passed in linked list of commands.
    while (command != NULL) {
        Command* next = command->next;
        if (command->val_a.type == OP_STR && command->val_a.as.str != NULL) {
            free(command->val_a.as.str);
            command->val_a.as.str = NULL;
        }
        free(command);
        command = next;
    }
}

void print_command(Command* cmd) {
    printf("Command type: %u\n", cmd->type);
    if (cmd->destination.type == OP_VAR) {
        printf("Destination: %" PRId64 "\n", cmd->destination.as.var);
    } else {
        printf("Destination: <invalid>\n");
    }
    printf("Operands:\n");
    printf("A:\n");
    print_command_op(&cmd->val_a);
    printf("\n");
    printf("B:\n");
    print_command_op(&cmd->val_b);
    printf("\n");
    printf("Branch condition: %u\n", cmd->branch_condition);
    printf("\n\n");
}

void print_command_op(const Operand* op) {
    printf("Type: %u\n", op->type);
    printf("Value: ");
    switch (op->type) {
        case OP_VAR:
            printf("x%" PRId64, op->as.var);
            break;
        case OP_IMM:
            printf("%" PRId64, op->as.imm);
            break;
        case OP_STR:
            printf("%s", op->as.str ? op->as.str : "<null>");
            break;
        case OP_BASE:
            printf("%c", op->as.base);
            break;
        default:
            printf("<unset>");
            break;
    }
    printf("\n");
}

void print_commands(Command* cmd) {
    if (!cmd) {
        printf("No commands found.\n");
    }

    while (cmd) {
        print_command(cmd);
        cmd = cmd->next;
        if (cmd) {
            printf("\n");
        }
    }
}
