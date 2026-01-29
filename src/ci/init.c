/* CS429 CI Lab
 */

#include "init.h"

#include <stdio.h>

#include "command.h"
#include "interpreter.h"
#include "lexer.h"
#include "mem.h"
#include "parser.h"

int ci_run_source(const char* src, bool print_lex, bool print_parse) {
    Lexer l;
    lexer_init(&l, src);
    if (print_lex) {
        print_lexed_tokens(&l);
        /* Reset so we can parse. */
        lexer_init(&l, src);
    }

    Parser p;
    parser_init(&p, &l);
    Command* commands = parse_commands(&p);
    if (print_parse) {
        print_commands(commands);
    }

    if (p.had_error) {
        printf("Parser encountered an error:\n");
        printf("At ");
        print_token(p.current);
        printf("\nParsed commands up to this point:\n");
        print_commands(commands);
        free_command(commands);  //  added
        return -1;
    }

    Interpreter i;
    interpreter_init(&i);
    interpret(&i, commands);
    print_interpreter_state(&i);
    mem_print();
    free_command(commands);  // added

    return (i.had_error) ? -1 : 0;
}
