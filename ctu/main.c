#include "util/str.h"
#include "util/report.h"
#include "ast/compile.h"
#include "sema/sema.h"
#include "debug/ast.h"
#include "debug/ir.h"
#include "ir/ir.h"
#include "speed/speed.h"
#include "gen/c99.h"
#include "gen/x86.h"
#include "ctu/util/util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* current name of this program */
static const char *name = NULL;

/* enable eager error reporting */
static bool eager_reporting = false;

/* should code be optimized */
static bool speed = false;

/* should the ir be dumped */
static bool emit = false;

/* should we print c99 or generate native x86 binaries */
static bool c99 = false;

typedef struct {
    const char *path;
    FILE *file;
} input_t;

typedef struct {
    input_t *files;
    size_t len, size;
} inputs_t;

static inputs_t inputs = { NULL, 0, 0 };

static void init_inputs(void) {
    inputs.files = ctu_malloc(sizeof(input_t) * 4);
    inputs.len = 0;
    inputs.size = 4;
}

static void add_file(const char *path) {
    FILE *file = fopen(path, "rb");
    ENSURE(file != NULL)("failed to open file `%s`", path);

    if (inputs.len + 1 > inputs.size) {
        inputs.size += 4;
        inputs.files = ctu_realloc(inputs.files, sizeof(input_t) * inputs.size);
    }
    input_t item = { path, file };
    inputs.files[inputs.len++] = item;
}

#define HELP_ARG "--help"
#define EAGER_ARG "--eager"
#define VERBOSE_ARG "--verbose"
#define OPTIMIZE_ARG "--speed"
#define EMIT_ARG "--emit"
#define C99_ARG "--c99"

static void print_help(void) {
    printf("usage: %s [options] file...\n", name);
    printf("options:\n");
    printf("\t" HELP_ARG ": print this message\n");
    printf("\t" EAGER_ARG ": enable eager error reporting\n");
    printf("\t" VERBOSE_ARG ": enable verbose logging\n");
    printf("\t" OPTIMIZE_ARG ": enable optimization\n");
    printf("\t" EMIT_ARG ": print debug info\n");
    printf("\t" C99_ARG ": enable C99 output\n");
}

static int parse_arg(int index, int argc, const char **argv) {
    (void)argc;
    const char *arg = argv[index];

    if (strcmp(arg, HELP_ARG) == 0) {
        print_help();
        exit(0);
    } else if (strcmp(arg, EAGER_ARG) == 0) {
        eager_reporting = true;
        logfmt("enabled eager reporting");
    } else if (strcmp(arg, VERBOSE_ARG) == 0) {
        verbose = true;
        logfmt("enabled verbose logging");
    } else if (strcmp(arg, OPTIMIZE_ARG) == 0) {
        speed = true;
        logfmt("enabled optimization");
    } else if (strcmp(arg, EMIT_ARG) == 0) {
        emit = true;
        logfmt("enabled ir debugging");
    } else if (strcmp(arg, C99_ARG) == 0) {
        c99 = true;
        logfmt("emitting c99");
    } else if (!startswith(arg, "-")) {
        add_file(arg);
        logfmt("adding `%s` as a source file", arg);
    } else {
        ensure("unknown argument `%s`", arg);
    }

    return 1;
}

static void parse_argc_argv(int argc, const char **argv) {
    if (argc == 1) {
        print_help();
        exit(0);
    }

    int index = 1;
    while (index < argc) {
        index += parse_arg(index, argc, argv);
    }
}

/**
 * open a file for writing and log if it fails to open 
 * then return NULL or the opened file
 */
static FILE *open_file(const char *path) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        ensure("failed to open `%s` for writing", path);
        return NULL;
    }
    logfmt("opened `%s` for writing", path);
    return file;
}

int main(int argc, const char **argv) {
    name = argv[0];

    init_inputs();
    report_begin(20, false);

    parse_argc_argv(argc, argv);

    if (report_end("input"))
        return 1;

    report_begin(20, eager_reporting);

    if (inputs.len == 0) {
        fprintf(stderr, "no input files\n");
        return 1;
    }

    types_init();
    sema_init();

    for (size_t i = 0; i < inputs.len; i++) {
        scanner_t *scan;
        input_t it = inputs.files[i];
        list_t *nodes = compile_file(it.path, it.file, &scan);

        if (report_end("parse"))
            return 1;

        if (emit) {
            for (size_t i = 0; i < nodes->len; i++) {
                debug_ast(nodes->data[i]); printf("\n");
            }
        }

        unit_t unit = typecheck(nodes);

        if (report_end("semantic"))
            return 1;

        if (emit) {
            for (size_t i = 0; i < nodes->len; i++) {
                debug_ast(nodes->data[i]); printf("\n");
            }
        }

        module_t *mod = compile_module("ctu/main", unit);

        if (report_end("intermediate"))
            return 1;

        if (emit) {
            debug_module(*mod);
        }

        if (speed) {
            pass_t pass = new_pass(mod);

            while (run_pass(&pass));
        }

        if (emit) {
            debug_module(*mod);
        }

        if (report_end("optimize"))
            return 1;

        if (c99) {
            FILE *out = open_file("out.c");
            if (out) {
                gen_c99(out, mod);
            }
        } else {
            //FILE *out = open_file("out.o");
            //if (out) {
            //    gen_x86(out, mod);
            //}
        }

        if (report_end("generate"))
            return 1;

        free_scanner(scan);
    }

    return 0;
}
