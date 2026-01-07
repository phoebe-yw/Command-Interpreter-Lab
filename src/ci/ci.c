/* CS429 CI Lab
 *
 * Students: Do Not Modify
 */

/** @file
 *  @cond DOXYGEN_IGNORE
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argparse/argparse.h"
#include "init.h"

#define CAPACITY 50

typedef struct {
    bool print_lex;     /* Lex; do not parse. */
    bool print_parse;   /* Print result of parsing. Implicitly performs lexing. */
    bool repl;          /* Set when no arguments are supplied. */
    char* in_filename;  /* What are we running? */
    char* out_filename; /* File to output to. */
} CmdArgsConfig;

static int run_interpreter(const CmdArgsConfig* conf);
static char* run_repl(void);
static char* read_file(const char* path);
static char* ci_strdup(const char* s);
static void config_free(CmdArgsConfig* conf);

int main(int argc, char** argv) {
    CmdArgsConfig conf = {false, false, false, NULL, NULL};

    int print_lex = 0;
    int print_parse = 0;
    const char* in_filename = NULL;
    const char* out_filename = NULL;

    static const char* const usages[] = {
        "ci [options]",
        NULL,
    };

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("CI options"),
        OPT_BOOLEAN('l', NULL, &print_lex,
                    "print lexer output only (do not parse)", NULL, 0, 0),
        OPT_BOOLEAN('p', NULL, &print_parse,
                    "print parser output (implicitly performs lexing)", NULL, 0,
                    0),
        OPT_STRING('i', NULL, &in_filename, "specify input filename", NULL, 0, 0),
        OPT_STRING('o', NULL, &out_filename, "specify output filename", NULL, 0,
                   0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(
        &argparse, NULL,
        "\nIf no arguments are provided, the program will start in REPL mode.");

    int remaining = argparse_parse(&argparse, argc, (const char**)argv);

    conf.print_lex = (print_lex != 0);
    conf.print_parse = (print_parse != 0);

    if (in_filename == NULL && remaining > 0) {
        in_filename = argv[0];
    }

    if (in_filename != NULL) {
        conf.in_filename = ci_strdup(in_filename);
        if (conf.in_filename == NULL) {
            fprintf(stderr, "Failed to allocate space for input filename\n");
            config_free(&conf);
            return 1;
        }
    }

    if (out_filename != NULL) {
        conf.out_filename = ci_strdup(out_filename);
        if (conf.out_filename == NULL) {
            fprintf(stderr, "Failed to allocate space for output filename\n");
            config_free(&conf);
            return 1;
        }
    }

    if (argc <= 1 && conf.in_filename == NULL) {
        conf.repl = true;
    }

    FILE* file = NULL;
    if (conf.out_filename != NULL) {
        file = freopen(conf.out_filename, "w", stdout);
        if (file == NULL) {
            perror("Failed to redirect stdout");
            config_free(&conf);
            return 1;
        }
    }

    int status = run_interpreter(&conf);
    config_free(&conf);
    /* Don't close file, as it's stdout. */

    return status;
}

static void config_free(CmdArgsConfig* conf) {
    if (!conf) {
        return;
    }

    free(conf->in_filename);
    free(conf->out_filename);
    conf->in_filename = NULL;
    conf->out_filename = NULL;
}

static int run_interpreter(const CmdArgsConfig* conf) {
    char* src;
    int status;

    if (conf->repl) {
        src = run_repl();
        if (!src) {
            return -1;
        }
    } else {
        if (conf->in_filename == NULL) {
            printf("No file specified.\n");
            return -1;
        }
        src = read_file(conf->in_filename);
        if (!src) {
            return -1;
        }
    }

    status = ci_run_source(src, conf->print_lex, conf->print_parse);
    free(src);
    return status;
}

static char* run_repl(void) {
    char* buffer = (char*)malloc(CAPACITY * sizeof(char));
    if (!buffer) {
        printf("Could not allocate memory for REPL buffer\n");
        return NULL;
    }

    size_t total_size = CAPACITY;
    size_t current_size = 0;
    char line[256];
    bool continue_input = true;

    printf("Enter commands:\n");

    while (continue_input) {
        printf("CI> ");
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        size_t line_len = strlen(line);

        if (current_size + line_len >= total_size) {
            total_size *= 2;
            char* new_buffer = (char*)realloc(buffer, total_size);
            if (!new_buffer) {
                printf("Could not allocate more memory for REPL buffer\n");
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }

        strcpy(buffer + current_size, line);
        current_size += line_len;

        bool has_semicolon = false;
        for (int i = (int)line_len - 1; i >= 0; i--) {
            if (line[i] == ';') {
                has_semicolon = true;
                break;
            }
            if (!isspace((unsigned char)line[i])) {
                break;
            }
        }

        if (!has_semicolon) {
            continue_input = false;
        }
    }

    return buffer;
}

static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("Failed to open file %s\n", path);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t filesize = (size_t)ftell(file);
    rewind(file);

    char* buffer = (char*)calloc(filesize + 1, sizeof(char));
    if (!buffer) {
        printf("Could not allocate enough space for %s\n", path);
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, sizeof(char), filesize, file);
    if (bytes_read < filesize) {
        printf("Could not read %s\n", path);
    }

    fclose(file);
    return buffer;
}

static char* ci_strdup(const char* s) {
    if (s == NULL) {
        return NULL;
    }
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (out == NULL) {
        return NULL;
    }
    memcpy(out, s, len + 1);
    return out;
}

/** @endcond */