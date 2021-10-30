#include "cmd.h"

#include "ctu/util/util.h"
#include "ctu/util/str.h"

#include <stdlib.h>

static const char *NAME = "driver";

static void print_help(const frontend_t *frontend) {
    printf("%s (%s)\n", frontend->name, frontend->version);
    printf("Usage: %s [options...] [sources...]\n", NAME);
    printf("Options:\n");
    printf("\t -h, --help: Print this help message\n");
    printf("\t -v, --version: Print version information\n");
    printf("\t -gen, --generator: Override default backend code generator\n");
    printf("\t -V, --verbose: Enable verbose logging\n");
    printf("\t -ir, --intermediate: Print IR\n");
    printf("\t -O, --opt: set optimisation level\n");
    printf("\t -P, --pass: specify optimisation passes to run\n");
    printf("\t -o, --output: Specify output file\n");

    exit(0);
}

static void print_version(const frontend_t *frontend) {
    printf("%s (%s)\n", frontend->name, frontend->version);
    printf("Backends:\n");
    printf("* C99 Version: %s\n", BACKEND_C99.version);
    printf("* GCCJIT Version: %s\n", BACKEND_GCCJIT.version);
    printf("* LLVM Version: %s\n", BACKEND_LLVM.version);

    exit(0);
}

#define MATCH(arg, a, b) (startswith(arg, a) || startswith(arg, b))
#define NEXT(idx, argc, argv) (idx + 1 >= argc ? NULL : argv[idx + 1])

static int parse_arg(settings_t *settings, const frontend_t *frontend, int index, int argc, char **argv) {
    const char *arg = argv[index];
    
    if (!startswith(arg, "-")) {
        file_t *fp = ctu_fopen(arg, "rb");

        if (fp->file == NULL) {
            report(settings->reports, ERROR, NULL, "failed to open file: %s", arg);
        } else {
            vector_push(&settings->sources, fp);
        }
    } else if (MATCH(arg, "-h", "--help")) {
        print_help(frontend);
    } else if (MATCH(arg, "-v", "--version")) {
        print_version(frontend);
    } else if (MATCH(arg, "-gen", "--generator")) {
        if (settings->backend != NULL) {
            report(settings->reports, ERROR, NULL, "generator already specified");
        }

        settings->backend = select_backend(settings->reports, NEXT(index, argc, argv));
        return 2;
    } else if (MATCH(arg, "-V", "--verbose")) {
        settings->verbose = true;
    } else if (MATCH(arg, "-ir", "--intermediate")) {
        settings->ir = true;
    } else if (MATCH(arg, "-o", "--output")) {
        settings->output = NEXT(index, argc, argv);
        return 2;
    } else {
        report(settings->reports, WARNING, NULL, "unknown argument %s", arg);
    }

    return 1;
}

settings_t parse_args(reports_t *reports, const frontend_t *frontend, int argc, char **argv) {
    NAME = argv[0];

    settings_t settings = { 
        .backend = NULL,
        .sources = vector_new(0),
        .reports = reports,
        .verbose = false,
        .ir = false
    };

    if (argc == 1) {
        print_help(frontend);
    }

    for (int i = 1; i < argc;) {
        i += parse_arg(&settings, frontend, i, argc, argv);
    }

    return settings;
}
