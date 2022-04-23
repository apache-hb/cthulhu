#include "cthulhu/util/error.h"

#include "cthulhu/driver/driver.h"

#include "cthulhu/hlir/init.h"

#include "cthulhu/util/report.h"
#include "cthulhu/util/str.h"
#include "cthulhu/util/vector.h"
#include "cthulhu/ast/compile.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/emit/emit.h"
#include "cthulhu/loader/hlir.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define DEFAULT_REPORT_LIMIT 20

static void print_version(driver_t driver) {
    printf("%s: %s\n", driver.name, driver.version);
}

static void print_help(const char **argv) {
    printf("usage: %s <files & objects...> [options...]\n", argv[0]);
    printf("general options:\n");
    printf("  -h, --help        : print this help message\n");
    printf("  -v, --version     : print version information\n");
    printf("  -V, --verbose     : enable verbose logging\n");
    printf("  -m, --module      : set module output name\n");
    printf("  -out, --output    : set output file name\n");
    printf("  -t, --target      : set output format\n");
    printf("                    | options: c89, wasm\n");
    printf("                    | default: c89\n");
    printf("---\n");
    printf("warning options:\n");
    printf("  -Wlimit=<digit>   : set warning limit\n");
    printf("                    | options: 0 = infinite\n");
    printf("                    | default: %d\n", DEFAULT_REPORT_LIMIT);
    printf("---\n");
    printf("bytecode options:\n");
    printf("  -bc, --bytecode   : enable bytecode generation\n");
    printf("  -Bembed           : embed source in bytecode\n");
}

static bool streq_nullable(const char *lhs, const char *rhs) {
    return lhs != NULL && rhs != NULL && str_equal(lhs, rhs);
}

static bool find_arg(int argc, const char **argv, const char *arg, const char *brief) {
    for (int i = 0; i < argc; i++) {
        if (streq_nullable(argv[i], arg) || streq_nullable(argv[i], brief)) {
            return true;
        }
    }

    return false;
}

static const char *extract_arg(int argc, const char **argv, const char *arg, int idx) {
    size_t len = strlen(arg);
    if (str_startswith(argv[idx], arg)) {
        char end = argv[idx][len];
        if (end == '\0') {
            if (idx >= argc) {
                return NULL;
            }

            return argv[idx + 1];
        } 
        
        if (end == '=') {
            return argv[idx] + len + 1;
        } 

        return argv[idx] + len;
    }

    return NULL;
}

static const char *get_arg(reports_t *reports, int argc, const char **argv, const char *arg, const char *brief) {
    bool found = false;
    const char *result = NULL;

    for (int i = 0; i < argc; i++) {
        if (str_startswith(argv[i], arg)) {
            found = true;
            result = extract_arg(argc, argv, arg, i);
            break;
        }

        if (brief != NULL && str_startswith(argv[i], brief)) {
            found = true;
            result = extract_arg(argc, argv, brief, i);
            break;
        }
    }

    if (result == NULL && found) {
        report(reports, WARNING, NULL, "missing argument for %s", arg);
    }

    return result;
}

static vector_t *collect_args(int argc, const char **argv, const char *prefix, const char *brief) {
    vector_t *result = vector_new(argc - 1);

    for (int i = 1; i < argc; i++) {
        if (str_startswith(argv[i], prefix)) {
            const char *arg = extract_arg(argc, argv, prefix, i);
            if (arg != NULL) {
                vector_push(&result, (char*)arg);
                continue;
            }
        }

        if (brief != NULL && str_startswith(argv[i], brief)) {
            const char *arg = extract_arg(argc, argv, brief, i);
            if (arg != NULL) {
                vector_push(&result, (char*)arg);
                continue;
            }
        }
    }

    return result;
}

static void rename_module(reports_t *reports, hlir_t *hlir, const char *path, const char *mod) {
    if (mod != NULL && hlir->name != NULL) {
        message_t *id = report(reports, WARNING, NULL, "module name already defined in source file, overriding this may not be desired");
        report_note(id, "redefining `%s` to `%s`", hlir->name, mod);
    }

    if (hlir->name == NULL) {
        hlir->name = (mod != NULL) ? ctu_strdup(mod) : ctu_filename(path);
    }
}

static size_t get_limit(reports_t *reports, const char **argv, int argc) {
    const char *input = get_arg(reports, argc, argv, "-Wlimit", NULL);
    if (input == NULL) {
        return DEFAULT_REPORT_LIMIT;
    }

    mpz_t limit;
    mpz_init_set_str(limit, input, 10);
    if (!mpz_fits_uint_p(limit)) {
        report(reports, WARNING, NULL, "warning limit `%s` is too large, defaulting to %d", input, DEFAULT_REPORT_LIMIT);
        return DEFAULT_REPORT_LIMIT;
    }

    if (mpz_sgn(limit) < 0) {
        report(reports, WARNING, NULL, "warning limit `%s` is negative, defaulting to %d", input, DEFAULT_REPORT_LIMIT);
        return DEFAULT_REPORT_LIMIT;
    }

    size_t result = mpz_get_ui(limit);
    mpz_clear(limit);

    return result > 0 ? result : SIZE_MAX;
}

void common_init(void) {
    init_gmp();
    init_hlir();
}

typedef enum {
    OUTPUT_C89,
    OUTPUT_WASM
} target_t;

static target_t parse_target(reports_t *reports, const char *target) {
    if (str_equal(target, "c89")) {
        return OUTPUT_C89;
    }  

    if (str_equal(target, "wasm")) {
        return OUTPUT_WASM;
    }

    message_t *id = report(reports, WARNING, NULL, "unknown output target `%s`", target);
    report_note(id, "defaulting to `c89`");
    return OUTPUT_C89;
}

static char *collect_commandline(int argc, const char **argv) {
    CTASSERT(argc > 0, "argc must be greater than 0");
    vector_t *vec = vector_of(argc - 1);
    for (int i = 1; i < argc; i++) {
        vector_set(vec, i - 1, (char*)argv[i]);
    }
    char *out = str_join(" ", vec);
    vector_delete(vec);
    return out;
}

int common_main(int argc, const char **argv, driver_t driver) {
    char *args = collect_commandline(argc, argv);
    (void)args;
    (void)collect_args;

    if (find_arg(argc, argv, "--version", "-v")) {
        print_version(driver);
        return 0;
    }

    if (find_arg(argc, argv, "--help", "-h")) {
        print_help(argv);
        return 0;
    }

    if (find_arg(argc, argv, "--verbose", "-V")) {
        verbose = true;
    }

    reports_t *reports = begin_reports();
    int status = EXIT_INTERAL;

    if (argc < 2) {
        report(reports, ERROR, NULL, "no file specified");
        return end_reports(reports, SIZE_MAX, "command line parsing");
    }

    size_t limit = get_limit(reports, argv, argc);
    status = end_reports(reports, limit, "command line parsing");
    if (status != 0) { return status; }

    bool bytecode = find_arg(argc, argv, "--bytecode", "-bc");
    bool embed_bc = find_arg(argc, argv, "-Bembed", NULL);

    const char *mod_name = get_arg(reports, argc, argv, "--module", "-m");
    const char *out = get_arg(reports, argc, argv, "--output", "-out");
    if (out == NULL) {
        out = "a.out";
    }
    
    const char *target = get_arg(reports, argc, argv, "--target", "-t");

    if (target == NULL) { 
        target = "c89";
    } else if (bytecode) {
        report(reports, ERROR, NULL, "cannot specify target format when compiling to bytecode");
        return end_reports(reports, limit, "command line parsing");
    }
    
    target_t result = parse_target(reports, target);
    status = end_reports(reports, limit, "target parsing");
    if (status != 0) { return status; }

    const char *path = argv[1];

    file_t *file = file_new(path, TEXT, READ);
    if (!file_ok(file)) {
        ctu_errno_t error = ctu_last_error();
        report(reports, ERROR, NULL, "failed to open file: %s", ctu_err_string(error));
        return end_reports(reports, limit, "command line parsing");
    }

    // create our scanner, this must be retained for the duration of the program

    scan_t scan = scan_file(reports, driver.name, file);
    status = end_reports(reports, limit, "scanning");
    if (status != 0) { return status; }

    // parse the source file

    void *node = driver.parse(reports, &scan);
    status = end_reports(reports, limit, "parsing");
    if (status != 0) { return status; }
    CTASSERT(node != NULL, "driver.parse == NULL");

    // convert the language ast to hlir

    hlir_t *hlir = driver.sema(reports, node);
    status = end_reports(reports, limit, "semantic analysis");
    if (status != 0) { return status; }
    CTASSERT(hlir != NULL, "driver.sema == NULL");

    rename_module(reports, hlir, path, mod_name);
    check_module(reports, hlir);
    status = end_reports(reports, limit, "module checking");
    if (status != 0) { return status; }

    if (bytecode) {
        save_settings_t settings = { .embed_source = embed_bc };

        save_module(reports, &settings, hlir, out);
        status = end_reports(reports, limit, "bytecode generation");
        return status;
    }

    switch (result) {
    case OUTPUT_C89:
        c89_emit_tree(reports, hlir);
        status = end_reports(reports, limit, "emitting c89");
        break;
    case OUTPUT_WASM:
        wasm_emit_tree(reports, hlir);
        status = end_reports(reports, limit, "emitting wasm");
        break;
    default:
        report(reports, ERROR, NULL, "unknown target %d selected", result);
        status = end_reports(reports, limit, "emitting");
        break;
    }

    return status;
}

hlir_t *find_module(sema_t *sema, const char *path) {
    hlir_t *hlir = load_module(sema->reports, format("%s.hlir", path));
    if (hlir != NULL) { return hlir; }

    return NULL;
}
