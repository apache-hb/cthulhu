#include "cthulhu/util/error.h"

#include "cmd.h"
#include "cthulhu/driver/driver.h"


#include "cthulhu/hlir/init.h"

#include "cthulhu/ast/compile.h"
#include "cthulhu/emit/emit.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/loader/hlir.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/str.h"
#include "cthulhu/util/vector.h"
#include "src/driver/cmd.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static void print_version(driver_t driver) {
    printf("%s: %s\n", driver.name, driver.version);
}

static char *join_names(const char **names, size_t num) {
    vector_t *vec = vector_of(num);

    for (size_t i = 0; i < num; i++) {
        vector_set(vec, i, (char *)names[i]);
    }

    return str_join(", ", vec);
}

static void print_help(const char **argv) {
    printf("usage: %s <files... & objects... & options...>\n", argv[0]);

#define SECTION(id) printf("%s options:\n", id);
#define COMMAND(name, type, initial, description, ...)       \
    do {                                                     \
        const char *names[] = __VA_ARGS__;                   \
        size_t total = sizeof(names) / sizeof(const char *); \
        const char *parts = join_names(names, total);        \
        printf("  %-20s : %s\n", parts, description);        \
    } while (0);

#include "flags.inc"
#if 0
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
#endif
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

static void rename_module(reports_t *reports, hlir_t *hlir, const char *path, const char *mod) {
    if (mod != NULL && hlir->name != NULL) {
        message_t *id = report(reports, WARNING, NULL,
                               "module name already defined in source file, overriding this may not be desired");
        report_note(id, "redefining `%s` to `%s`", hlir->name, mod);
    }

    if (hlir->name == NULL) {
        hlir->name = (mod != NULL) ? ctu_strdup(mod) : ctu_filename(path);
    }
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

int common_main(int argc, const char **argv, driver_t driver) {
    reports_t *reports = begin_reports();
    commands_t commands = { 0 };
    int status = parse_commandline(reports, &commands, argc, argv);
    if (status != 0) { return status; }

    verbose = commands.verboseLogging;
    size_t limit = commands.warningLimit;

    if (commands.printHelp) {
        print_help(argv);
        return 0;
    }

    if (commands.printVersion) {
        print_version(driver);
        return 0;
    }

    const char *out = get_arg(reports, argc, argv, "--output", "-out");
    if (out == NULL) {
        out = "a.out";
    }

    const char *target = get_arg(reports, argc, argv, "--target", "-t");

    if (target == NULL) {
        target = "c89";
    } else if (commands.enableBytecode) {
        report(reports, ERROR, NULL, "cannot specify target format when compiling to bytecode");
        return end_reports(reports, limit, "command line parsing");
    }

    target_t result = parse_target(reports, target);
    status = end_reports(reports, limit, "target parsing");
    if (status != 0) {
        return status;
    }

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
    if (status != 0) {
        return status;
    }

    // parse the source file

    void *node = driver.parse(reports, &scan);
    status = end_reports(reports, limit, "parsing");
    if (status != 0) {
        return status;
    }
    CTASSERT(node != NULL, "driver.parse == NULL");

    // convert the language ast to hlir

    hlir_t *hlir = driver.sema(reports, node);
    status = end_reports(reports, limit, "semantic analysis");
    if (status != 0) {
        return status;
    }
    CTASSERT(hlir != NULL, "driver.sema == NULL");

    rename_module(reports, hlir, path, commands.moduleName);
    check_module(reports, hlir);
    status = end_reports(reports, limit, "module checking");
    if (status != 0) {
        return status;
    }

    if (commands.enableBytecode) {
        save_settings_t settings = {.embedSource = commands.embedSource };

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
    if (hlir != NULL) {
        return hlir;
    }

    return NULL;
}
