/* CS429 CI Lab
 *
 * Students: Do Not Modify
 */

#ifndef CI_INIT_H
#define CI_INIT_H

#include <stdbool.h>

/**
 * @brief Runs the CI pipeline on the given source string.
 *
 * @param src The CI source program to execute.
 * @param print_lex If true, prints lexer output (and still parses afterwards).
 * @param print_parse If true, prints the parsed command list.
 * @return 0 on success, -1 on interpreter or parse error.
 */
int ci_run_source(const char *src, bool print_lex, bool print_parse);

#endif
