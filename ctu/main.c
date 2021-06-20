#include "util/str.h"
#include "util/report.h"
#include "ast/compile.h"
#include "debug/ast.h"

#include <string.h>
#include <stdio.h>

/* current name of this program */
static const char *name = NULL;

/* name of file to output to */
static const char *output = "a.out";

#define HELP_ARG "--help"
#define OUTPUT_ARG "--output"

static void print_help(void) {
    printf("usage: %s [options] file...\n", name);
    printf("options:\n");
    printf("\t" HELP_ARG ": print this message\n");
    printf("\t" OUTPUT_ARG "=name: set output name (default %s)\n", output);
}

static int parse_arg(int index, int argc, const char **argv) {
    (void)argc;
    const char *arg = argv[index];

    if (strcmp(arg, HELP_ARG) == 0) {
        print_help();
    }

    return 1;
}

static void parse_argc_argv(int argc, const char **argv) {
    if (argc == 1) {
        print_help();
    }

    int index = 1;
    while (index < argc) {
        index += parse_arg(index, argc, argv);
    }
}

int main(int argc, const char **argv) {
    name = argv[0];
    parse_argc_argv(argc, argv);

    report_begin(20);

    nodes_t *nodes = compile_string("stdin", 
        "{ 5; 6; lol return 69; }"
    );

    if (report_end("parse"))
        return 1;

    for (size_t i = 0; i < nodes->len; i++) {
        debug_ast(nodes->data + i);
        printf("\n");
    }

    return 0;
}
