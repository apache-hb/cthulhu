#include "cthulhu/util/error.h"

#include "cmd.h"
#include "cthulhu/driver/driver.h"
#include "cthulhu/hlir/init.h"
#include "cthulhu/util/io.h"
#include "plugins.h"

#include "cthulhu/ast/compile.h"
#include "cthulhu/emit/emit.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/loader/hlir.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/str.h"
#include "cthulhu/util/vector.h"
#include "src/driver/cmd.h"
#include "src/driver/plugins.h"

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
}

static void rename_module(reports_t *reports, hlir_t *hlir, const char *path) {
    if (hlir->name != NULL) { return; }
    hlir->name = ctu_filename(path);
}

void common_init(void) {
    init_gmp();
    init_hlir();
}

typedef struct {
    file_t *file;
    scan_t scanner;
    void *node;
    hlir_t *hlir;
} context_t;

typedef enum { OUTPUT_C89, OUTPUT_WASM } target_t;

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
    if (status != 0) {
        return status;
    }

    verbose = commands.verboseLogging;

    vector_t *files = commands.files;
    size_t limit = commands.warningLimit;
    const char *target = commands.outputTarget;
    const char *outFile = commands.outputFile;

    if (outFile == NULL) {
        outFile = commands.enableBytecode ? "mod.hlir" : "a.out";
    }

    if (commands.printHelp) {
        print_help(argv);
        return 0;
    }

    if (commands.printVersion) {
        print_version(driver);
        return 0;
    }

    vector_t *plugins = vector_new(0); // all plugins
    vector_t *modules = vector_new(0); // all library bytecode modules
    vector_t *sources = vector_new(0); // all source files

    for (size_t i = 0; i < vector_len(files); i++) {
        const char *path = vector_get(files, i);
        plugin_handle_t *handle = is_plugin(path);
        if (handle != NULL) {
            vector_push(&plugins, handle);
            continue;
        }

        if (is_hlir_module(path)) {
            vector_push(&modules, (char *)path);
            continue;
        }

        vector_push(&sources, (char *)path);
    }

    for (size_t i = 0; i < vector_len(plugins); i++) {
        plugin_handle_t *handle = vector_get(plugins, i);
        if (!plugin_load(reports, handle)) {
            continue;
        }

        if (handle->init != NULL) {
            plugin_t plugin = { .reports = reports };
            handle->init(&plugin);
        }
    }

    target_t result = parse_target(reports, target);
    status = end_reports(reports, limit, "target parsing");
    if (status != 0) {
        return status;
    }

    size_t numSources = vector_len(sources);
    vector_t *contexts = vector_of(numSources);

    for (size_t i = 0; i < numSources; i++) {
        const char *path = vector_get(sources, i);

        context_t *ctx = ctu_malloc(sizeof(context_t));

        file_t *file = file_new(path, TEXT, READ);
        if (!file_ok(file)) {
            ctu_errno_t err = ctu_last_error();
            report(reports, ERROR, NULL, "failed to open file: %s", ctu_err_string(err));
            continue;
        }

        ctx->file = file;
        vector_set(contexts, i, ctx);
    }

    status = end_reports(reports, limit, "opening files");
    if (status != 0) {
        return status;
    }

    for (size_t i = 0; i < numSources; i++) {
        context_t *ctx = vector_get(contexts, i);
        ctx->scanner = scan_file(reports, driver.name, ctx->file);
    }

    status = end_reports(reports, limit, "scanning files");
    if (status != 0) {
        return status;
    }

    for (size_t i = 0; i < numSources; i++) {
        context_t *ctx = vector_get(contexts, i);
        ctx->node = driver.parse(reports, &ctx->scanner);
    }

    status = end_reports(reports, limit, "parsing");
    if (status != 0) {
        return status;
    }

    for (size_t i = 0; i < numSources; i++) {
        context_t *ctx = vector_get(contexts, i);
        CTASSERT(ctx->node != NULL, "node should not be NULL");
        ctx->hlir = driver.sema(reports, ctx->node);
    }

    status = end_reports(reports, limit, "semantic analysis");
    if (status != 0) {
        return status;
    }

    for (size_t i = 0; i < numSources; i++) {
        context_t *ctx = vector_get(contexts, i);
        const char *path = vector_get(sources, i);
        rename_module(reports, ctx->hlir, path);
        check_module(reports, ctx->hlir);
    }

    status = end_reports(reports, limit, "module checking");
    if (status != 0) {
        return status;
    }

    if (commands.enableBytecode) {
        vector_t *bytecodeModules = vector_of(numSources);
        for (size_t i = 0; i < numSources; i++) {
            context_t *ctx = vector_get(contexts, i);
            vector_set(bytecodeModules, i, ctx->hlir);
        }

        save_settings_t settings = { .embedSource = commands.embedSource };

        save_modules(reports, &settings, bytecodeModules, outFile);
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
    vector_t *modules = load_modules(sema->reports, format("%s.hlir", path));
    size_t numModules = vector_len(modules);

    return NULL;
}
