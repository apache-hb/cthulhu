#include "report/report.h"
#include "front/front.h"
#include "middle/middle.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static const char *name = NULL;
static const char *program = NULL;

static bool print_ast = false;
//static bool print_ir = false;

static void print_help(void) {
    printf("%s: cthulhu compiler\n", name);
    puts("\t--help: display help message");
    puts("\t--print-ast: print ast of input");
    exit(0);
}

static void check_errors(const char *stage) {
    uint64_t total = errors();

    if (total > 0) {
        reportf("%s: aborting due to %llu previous error(s)", stage, total);
        exit(1);
    }
}

static void parse_arg(int index, const char **argv) {
    const char *current = argv[index];

    if (strcmp(current, "--help") == 0) {
        print_help();
    } else if (strcmp(current, "--print-ast") == 0) {
        print_ast = true;
    } else {
        program = current;
    }
}

int main(int argc, const char **argv) {
    name = argv[0];

    if (1 > argc) {
        print_help();
    }

    for (int i = 1; i < argc; i++) {
        parse_arg(i, argv);
    }

    if (!program) {
        print_help();
    }

    nodes_t *nodes = compile_string("string", program);
    
    check_errors("parsing");
    
    if (print_ast) {
        debug_t debug = { stdout };

        for (size_t i = 0; i < nodes->len; i++) {
            debug_ast(&debug, nodes->data + i);
            printf("\n");
        }
    }

    return 0;
}
