#include "util/str.h"
#include "util/report.h"
#include "ast/compile.h"
#include "sema/sema.h"
#include "debug/ast.h"
#include "debug/ir.h"
#include "ir/ir.h"
#include "speed/speed.h"
#include "gen/x86.h"
#include "gen/elf.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* current name of this program */
static const char *name = NULL;

/* name of file to output to */
static const char *output = "a.out";

/* enable eager error reporting */
static bool eager_reporting = false;

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
    inputs.files = malloc(sizeof(input_t) * 4);
    inputs.len = 0;
    inputs.size = 4;
}

static void add_file(const char *path) {
    FILE *file = fopen(path, "r");
    ENSURE(file != NULL)("failed to open file `%s`", path);

    if (inputs.len + 1 > inputs.size) {
        inputs.size += 4;
        inputs.files = realloc(inputs.files, sizeof(input_t) * inputs.size);
    }
    input_t item = { path, file };
    inputs.files[inputs.len++] = item;
}

#define HELP_ARG "--help"
#define EAGER_ARG "--eager"
#define OUTPUT_ARG "--output"
#define VERBOSE_ARG "--verbose"

static void print_help(void) {
    printf("usage: %s [options] file...\n", name);
    printf("options:\n");
    printf("\t" HELP_ARG ": print this message\n");
    printf("\t" EAGER_ARG ": enable eager error reporting\n");
    printf("\t" OUTPUT_ARG "=name: set output name (default %s)\n", output);
    printf("\t" VERBOSE_ARG ": enable verbose logging\n");
}

static int parse_arg(int index, int argc, const char **argv) {
    (void)argc;
    const char *arg = argv[index];

    if (strcmp(arg, HELP_ARG) == 0) {
        print_help();
    } else if (strcmp(arg, EAGER_ARG) == 0) {
        eager_reporting = true;
    } else if (strcmp(arg, VERBOSE_ARG) == 0) {
        verbose = true;
        logfmt("enabled verbose logging");
    } else if (!startswith(arg, "-")) {
        add_file(arg);
        logfmt("adding `%s` as a source file", arg);
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

    sema_init();

    for (size_t i = 0; i < inputs.len; i++) {
        scanner_t *scan;
        input_t it = inputs.files[i];
        nodes_t *nodes = compile_file(it.path, it.file, &scan);

        if (report_end("parse"))
            return 1;

        typecheck(nodes);

        if (report_end("semantic"))
            return 1;

        module_t mod = compile_module(nodes);

        if (report_end("intermediate"))
            return 1;

        debug_module(mod);

        size_t passes = 0;

        while (true) {
            logfmt("beginning optimization pass %zu", passes + 1);
            size_t dirty_stages = 0;

            if (remove_dead_code(&mod)) {
                logfmt("removed dead code");
                dirty_stages += 1;
            }

            if (remove_unused_blocks(&mod)) {
                logfmt("removed unused blocks");
                dirty_stages += 1;
            }

            if (mem2reg(&mod)) {
                logfmt("reduced memory");
                dirty_stages += 1;
            }

            if (propogate_consts(&mod)) {
                logfmt("propogated values");
                dirty_stages += 1;
            }

            if (remove_unused_code(&mod)) {
                logfmt("removed unreferenced vregs");
                dirty_stages += 1;
            }

            if (remove_empty_blocks(&mod)) {
                logfmt("removed empty blocks");
                dirty_stages += 1;
            }

            if (remove_branches(&mod)) {
                logfmt("removed excess branches");
                dirty_stages += 1;
            }

            if (remove_jumps(&mod)) {
                logfmt("removed excess jumps");
                dirty_stages += 1;
            }

            if (remove_pure_code(&mod)) {
                logfmt("removed unused pure operations");
                dirty_stages += 1;
            }

            if (fold_consts(&mod)) {
                logfmt("folded constant expressions");
                dirty_stages += 1;
            }

            if (dirty_stages == 0) {
                break;
            }

            passes += 1;
        }

        if (report_end("optimize"))
            return 1;

        debug_module(mod);

        //gen_x64(&mod);

        //if (report_end("generate"))
        //    return 1;

        free_scanner(scan);
    }

    FILE *test = fopen("test.elf", "wb");
    emit_elf(test);
    fclose(test);

    return 0;
}
