#include "cthulhu/driver/driver.h"

#include "cthulhu/util/str.h"
#include "cthulhu/ast/compile.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/emit/emit.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

static void print_version(driver_t driver) {
    printf("%s: %s\n", driver.name, driver.version);
}

static void print_help(const char **argv) {
    printf("usage: %s <file> [options...]\n", argv[0]);
    printf("options:\n");
    printf("  -h, --help        : print this help message\n");
    printf("  -v, --version     : print version information\n");
    printf("  -V, --verbose     : enable verbose logging\n");
    printf("  -m, --module      : set module output name\n");
    //printf("  -dh, --debug-hlir : print HLIR debug tree\n");
    //printf("  -ds, --debug-ssa  : print SSA debug tree\n");
    printf("  -out, --output    : set output format\n");
    printf("                    | options: json, c89\n");
    printf("                    | default: c89\n");
}

static bool find_arg(int argc, const char **argv, const char *arg, const char *brief) {
    for (int i = 0; i < argc; i++) {
        if (streq(argv[i], arg) || streq(argv[i], brief)) {
            return true;
        }
    }

    return false;
}

static const char *extract_arg(int argc, const char **argv, const char *arg, int idx) {
    size_t len = strlen(arg);
    if (startswith(argv[idx], arg)) {
        char end = argv[idx][len];
        if (end == '\0') {
            if (idx >= argc) {
                return NULL;
            }

            return argv[idx + 1];
        } else if (end == '=') {
            return argv[idx] + len + 1;
        } else {
            return argv[idx] + len;
        }
    }

    return NULL;
}

static const char *get_arg(reports_t *reports, int argc, const char **argv, const char *arg, const char *brief) {
    bool found = false;
    const char *result = NULL;

    for (int i = 0; i < argc; i++) {
        if (startswith(argv[i], arg)) {
            found = true;
            result = extract_arg(argc, argv, arg, i);
            break;
        }

        if (startswith(argv[i], brief)) {
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

static void rename_module(reports_t *reports, hlir_t *hlir, const char *path, const char *mod) {
    if (mod != NULL && hlir->name != NULL) {
        message_t *id = report(reports, WARNING, NULL, "module name already defined in source file, overriding this may not be desired");
        report_note(id, "redefining `%s` to `%s`", hlir->name, mod);
    }

    if (hlir->name == NULL) {
        hlir->name = (mod != NULL) ? ctu_strdup(mod) : ctu_filename(path);
    }
}

void common_init(void) {
    init_gmp();
}

typedef enum {
    OUTPUT_C89,
    OUTPUT_JSON
} output_t;

static output_t parse_target(reports_t *reports, const char *target) {
    if (streq(target, "c89")) {
        return OUTPUT_C89;
    } else if (streq(target, "json")) {
        return OUTPUT_JSON;
    } else {
        message_t *id = report(reports, WARNING, NULL, "unknown output target `%s`", target);
        report_note(id, "defaulting to `c89`");
        return OUTPUT_C89;
    }
}

int common_main(int argc, const char **argv, driver_t driver) {
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
    int status = 99;

    if (argc < 2) {
        report(reports, ERROR, NULL, "no file specified");
        return end_reports(reports, SIZE_MAX, "command line parsing");
    }

    init_hlir();

    const char *mod_name = get_arg(reports, argc, argv, "--module", "-m");
    const char *out = get_arg(reports, argc, argv, "--output", "-out");
    if (out == NULL) { out = "c89"; }
    
    output_t target = parse_target(reports, out);
    status = end_reports(reports, SIZE_MAX, "target parsing");
    if (status != 0) { return status; }

    const char *path = argv[1];

    file_t file = ctu_fopen(path, "rb");
    if (!file_valid(&file)) {
        report(reports, ERROR, NULL, "failed to open file: %s (errno %d)", strerror(errno), errno);
        return end_reports(reports, SIZE_MAX, "command line parsing");
    }

    // create our scanner, this must be retained for the duration of the program

    scan_t scan = scan_file(reports, driver.name, &file);
    status = end_reports(reports, SIZE_MAX, "scanning");
    if (status != 0) { return status; }

    // parse the source file

    void *node = driver.parse(reports, &scan);
    status = end_reports(reports, SIZE_MAX, "parsing");
    if (status != 0) { return status; }
    CTASSERT(node != NULL, "driver.parse == NULL");

    // convert the language ast to hlir

    hlir_t *hlir = driver.sema(reports, node);
    status = end_reports(reports, SIZE_MAX, "semantic analysis");
    if (status != 0) { return status; }
    CTASSERT(hlir != NULL, "driver.sema == NULL");

    rename_module(reports, hlir, path, mod_name);
    check_module(reports, hlir);
    status = end_reports(reports, SIZE_MAX, "module checking");
    if (status != 0) { return status; }

    switch (target) {
    case OUTPUT_C89:
        c89_emit_tree(reports, hlir);
        status = end_reports(reports, SIZE_MAX, "emitting c89");
        break;
    case OUTPUT_JSON:
        json_emit_tree(reports, hlir);
        status = end_reports(reports, SIZE_MAX, "emitting json");
        break;
    default:
        UNREACHABLE();
    }

    return status;
}
