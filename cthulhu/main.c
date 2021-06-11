#include "report/report.h"
#include "front/front.h"
#include "middle/middle.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

static const char *name = NULL;
static const char *path = NULL;
static const char *expr = NULL;

static bool print_ast = false;
static bool print_ir = false;

static void print_help(void) {
    printf("%s: cthulhu compiler\n", name);
    puts("\t--help: display help message");
    puts("\t--print-ast: print text representation of ast");
    puts("\t--print-ir: print text representation of ir");
    puts("\t--expr <expr>: compile an expression");
    exit(0);
}

static void check_errors(const char *stage) {
    uint64_t total = errors();

    if (total > 0) {
        reportf("%s: aborting due to %llu previous error(s)", stage, total);
        exit(1);
    }
}

#define HAS_NEXT (index < argc - 1)
#define GET_NEXT (argv[index + 1])

static int parse_arg(int index, int argc, char **argv) {
    const char *current = argv[index];
    int step = 1;

    if (strcmp(current, "--help") == 0) {
        print_help();
    } else if (strcmp(current, "--print-ast") == 0) {
        print_ast = true;
    } else if (strcmp(current, "--print-ir") == 0) {
        print_ir = true;
    } else if (access(current, F_OK) == 0) {
        path = current;
    } else if (strcmp(current, "--expr") == 0) {
        if (HAS_NEXT) {
            expr = GET_NEXT;
            step += 1;
        } else {
            reportf("`--expr` requires an expression");
        }
    } else {
        fprintf(stderr, "unknown arg `%s`\n", current);
    }

    return step;
}

static nodes_t *compile_input(void) {
    if (expr) {
        return compile_string("expr", expr);
    }

    FILE *file = fopen(path, "r");

    if (!file) {
        reportf("failed to open %s", path);
        return NULL;
    }

    return compile_file(path, file);
}

int compile_main(void) {
    debug_t debug = { stdout };

    nodes_t *nodes = compile_input();
    check_errors("parsing");
    
    if (print_ast) {
        for (size_t i = 0; i < nodes->len; i++) {
            debug_ast(&debug, nodes->data + i);
            printf("\n");
        }
    }

    unit_t unit = transform_ast(nodes);
    check_errors("generating ir");

    if (print_ir) {
        debug_unit(&debug, &unit);
    }

    return 0;
}

int main(int argc, char **argv) {
    name = argv[0];

#if CTC_FUZZING
    (void)parse_arg;
    if (1 > argc) {
        report("incorrect argc count for fuzzing");
        return 0;
    }
        
    program = argv[1];
    print_ast = true;
    print_ir = true;
#else
    if (argc == 1) {
        print_help();
    }

    int i = 1;
    while (i < argc) {
        i += parse_arg(i, argc, argv);
    }

    if (!path && !expr) {
        print_help();
    }
#endif

    return compile_main();
}
