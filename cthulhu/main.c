#include "config.h"

#include "report/report.h"
#include "front/front.h"
#include "middle/middle.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "back/llvm/llvm.h"
#include "back/llvm/debug.h"

static void check_errors(const char *stage) {
    uint64_t total = errors();

    if (total > 0) {
        reportf("%s: aborting due to %llu previous error(s)", stage, total);
        exit(1);
    }
}

static const char *name = NULL;
static const char *path = NULL;
static const char *expr = NULL;
static const char *output = "a.out";

static bool print_ast = false;
static bool print_ir = false;
static bool print_backend = false;

typedef enum {
    /* use our own codegen */
    LOCAL,
    /* use llvm for codegen */
    LLVM,
    /* use gcc for codegen */
    GCC,
    /* use qbe for codegen */
    QBE
} backend_t;

static backend_t get_backend(const char *arg) {
    if (strcmp(arg, "local") == 0)
        return LOCAL;

    if (strcmp(arg, "llvm") == 0)
        return LLVM;

    if (strcmp(arg, "gcc") == 0)
        return GCC;

    if (strcmp(arg, "qbe") == 0)
        return QBE;

    reportf("unknown backend `%s`", arg);
    return LOCAL;
}

static backend_t backend = LOCAL;

static void backend_compile(debug_t *debug, unit_t *unit) {
    void *context;
    FILE *file = fopen(output, "w");

    if (!file) {
        reportf("failed to open output file `%s`", output);
        return;
    }

    switch (backend) {
    case LLVM:
#if CTU_LLVM
        context = llvm_compile(unit);
        check_errors("llvm compile");

        if (print_backend) {
            llvm_debug(debug, context);
            check_errors("llvm debug");
        }

        llvm_output(context, file);
        check_errors("llvm output");
#else
        report("llvm backend is disabled");
#endif
        break;

    default:
        reportf("backend_compile(backend = %d)\n", backend);
        break;
    }
}

static const char *status(bool installed) {
    return installed ? "installed" : "not installed";
}

static void print_help(void) {
    printf("%s: cthulhu compiler\n", name);
    puts("\t--help: display help message");
    puts("\t--print-ast: print text representation of ast (default false)");
    puts("\t--print-ir: print text representation of ir (default false)");
    puts("\t--print-back: print text representation of backend (default false)");
    puts("\t--expr <expr>: compile an expression");
    puts("\t--output <path>: change output path (default a.out)");
    puts("\t--backend <backend>: select a backend, valid options are [local | llvm | gcc | qbe] (default local)");
    puts("installed backends:");
    printf("\tlocal: %s\n", status(true));
    printf("\tllvm: %s\n", status(CTU_LLVM));
    printf("\tgcc: %s\n", status(CTU_GCC));
    printf("\tqbe: %s\n", status(CTU_QBE));
    exit(0);
}

#define HAS_NEXT (index < argc - 1)
#define GET_NEXT (argv[index + 1])

static const char *get_next(
    const char *arg, int index, 
    int argc, char **argv,
    int *step
) {
    const char *current = argv[index];
    size_t arglen = strlen(arg);
    size_t curlen = strlen(current);

    if (curlen > arglen) {
        current += arglen + 1;
        return current[0] == '=' ? current + 1 : current;
    } else if (HAS_NEXT) {
        *step += 1;
        return GET_NEXT;
    } else {
        reportf("argument `%s` requires a parameter", arg);
        return NULL;
    }
}

#define EXPR "--expr"
#define BACKEND "--backend"
#define OUTPUT "--output"

static bool startswith(const char *str, const char *sub) {
    return strncmp(str, sub, strlen(sub)) == 0;
}

static int parse_arg(int index, int argc, char **argv) {
    const char *current = argv[index];
    int step = 1;

    if (startswith(current, "--help")) {
        print_help();
    } else if (startswith(current, "--print-ast")) {
        print_ast = true;
    } else if (startswith(current, "--print-ir")) {
        print_ir = true;
    } else if (startswith(current, "--print-backend")) {
        print_backend = true;
    } else if (startswith(current, OUTPUT)) {
        output = get_next(OUTPUT, index, argc, argv, &step);
    } else if (startswith(current, EXPR)) {
        expr = get_next(EXPR, index, argc, argv, &step);
    } else if (startswith(current, BACKEND)) {
        const char *next = get_next(BACKEND, index, argc, argv, &step);
        if (next) {
            backend = get_backend(next);
        }
    } else if (access(current, F_OK) == 0) {
        // TODO: there has to be a better way of checking if something 
        // is a file we want to parse
        path = current;
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
    check_errors("frontend");
    
    if (print_ast) {
        for (size_t i = 0; i < nodes->len; i++) {
            debug_ast(&debug, nodes->data + i);
            printf("\n");
        }
    }

    unit_t unit = transform_ast(nodes);
    check_errors("middle");

    if (print_ir) {
        debug_unit(&debug, &unit);
    }

    backend_compile(&debug, &unit);
    check_errors("backend");

    return 0;
}

int main(int argc, char **argv) {
    name = argv[0];

#if CTU_FUZZING
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

    check_errors("command line");
#endif

    return compile_main();
}
