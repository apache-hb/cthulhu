#include "cmd.h"

#include "cthulhu/util/util.h"
#include "cthulhu/util/str.h"

#include <stdlib.h>

static const char *NAME = "driver";

static const char *format_arg_type(arg_type_t type) {
    switch (type) {
    case ARG_BOOL: return "bool";
    case ARG_INT: return "int";
    case ARG_UINT: return "uint";
    default: return "none";
    }
}

static void print_help(const frontend_t *frontend) {
    printf("%s (%s)\n", frontend->name, frontend->version);
    printf("Usage: %s [options...] [sources...]\n", NAME);
    printf("Generic Options:\n");
    printf("\t -h, --help: Print this help message\n");
    printf("\t -v, --version: Print version information\n");
    printf("\t -gen, --generator: Override default backend code generator\n");
    printf("\t -V, --verbose: Enable verbose logging\n");
    printf("\t -ir, --intermediate: Print IR\n");
    printf("\t -O, --opt: set optimisation level\n");
    printf("\t -P, --pass: specify optimisation passes to run\n");
    printf("\t -o, --output: Specify output file\n");

    if (frontend->args != NULL) {
        size_t len = vector_len(frontend->args);
        printf("\n%zu Special Options:\n", len);
        for (size_t i = 0; i < len; i++) {
            const arg_t *arg = vector_get(frontend->args, i);
            printf("\t%s, %s=%s: %s\n", arg->name, arg->brief, format_arg_type(arg->type), arg->desc);
        }
    }

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

static int parse_user_arg(settings_t *settings, size_t index, const arg_t *arg, const char *part) {
    parsed_arg_t *parsed = ctu_malloc(sizeof(parsed_arg_t));
    parsed->type = arg->type;
    int skip = 2;
    printf("type: %s\n", format_arg_type(arg->type));
    switch (arg->type) {
    case ARG_BOOL:
        if (streq(part, "true") || streq(part, "1")) {
            parsed->boolean = true;
        } else if (streq(part, "false") || streq(part, "0")) {
            parsed->boolean = false;
        } else {
            report(settings->reports, WARNING, NULL, "invalid boolean value '%s', defaulting to false", part);
            parsed->boolean = false;
            skip = 1;
        }
        break;
    case ARG_INT:
        if (mpz_init_set_str(parsed->digit, part, 10) == -1) {
            report(settings->reports, WARNING, NULL, "invalid integer value '%s', defaulting to 0", part);
            mpz_set_si(parsed->digit, 0);
        }
        break;
    case ARG_UINT:
        if (mpz_init_set_str(parsed->digit, part, 10) == -1) {
            report(settings->reports, WARNING, NULL, "invalid unsigned integer value '%s', defaulting to 0", part);
            mpz_set_si(parsed->digit, 0);
        }
        if (mpz_sgn(parsed->digit) < 0) {
            report(settings->reports, WARNING, NULL, "unisnged integers cannot be more than 0 '%s', truncating to 0", part);
            mpz_set_si(parsed->digit, 0);
        }
        printf("uint: %s\n", mpz_get_str(NULL, 10, parsed->digit));
        break;
    default:
        ctu_assert(settings->reports, "invalid argument type");
        skip = 1;
        break;
    }

    vector_set(settings->extra, index, parsed);
    printf("set %zu\n", index);
    return skip;
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
        return 1;
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
        return 1;
    } else if (MATCH(arg, "-ir", "--intermediate")) {
        settings->ir = true;
        return 1;
    } else if (MATCH(arg, "-o", "--output")) {
        settings->output = NEXT(index, argc, argv);
        return 2;
    } else if (frontend->args != NULL) {
        size_t len = vector_len(frontend->args);
        for (size_t i = 0; i < len; i++) {
            const arg_t *arg = vector_get(frontend->args, i);
            printf("%s %zu\n", arg->name, i);
            if (MATCH(arg->name, arg->brief, arg->name)) {
                printf("match\n");
                return parse_user_arg(settings, i, arg, NEXT(index, argc, argv));
            }
        }
    }

    report(settings->reports, WARNING, NULL, "unknown argument %s", arg);
    return 1;
}

settings_t parse_args(reports_t *reports, const frontend_t *frontend, int argc, char **argv) {
    NAME = argv[0];

    settings_t settings = { 
        .backend = NULL,
        .sources = vector_new(0),
        .reports = reports,
        .verbose = false,
        .ir = false,
        .extra = vector_new(frontend->args == NULL ? 0 : vector_len(frontend->args))
    };

    if (argc == 1) {
        print_help(frontend);
    }

    for (int i = 1; i < argc;) {
        i += parse_arg(&settings, frontend, i, argc, argv);
    }

    return settings;
}

parsed_arg_t *get_arg(reports_t *reports, settings_t *settings, size_t idx, arg_type_t type) {
    printf("get %zu\n", idx);
    parsed_arg_t *arg = vector_get(settings->extra, idx);
    if (arg == NULL) {
        printf("NULL\n");
        return NULL;
    }

    if (arg->type != type) {
        printf("type: %s\n", format_arg_type(arg->type));
        ctu_assert(reports, "argument at %zu is of type %s, not %s", idx, format_arg_type(arg->type), format_arg_type(type));
        return NULL;
    }

    printf("got\n");
    return arg;
}
