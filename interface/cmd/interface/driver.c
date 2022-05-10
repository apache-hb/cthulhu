#include "cthulhu/driver/driver.h"
#include "cmd.h"
#include "cthulhu/hlir/init.h"
#include "cthulhu/util/version-def.h"
#include "plugins.h"

#include "cmd.h"
#include "cthulhu/ast/compile.h"
#include "cthulhu/emit/c89.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/str.h"
#include "cthulhu/util/vector.h"
#include "plugins.h"
#include "switch.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static void print_version(driver_t driver)
{
    int major = VERSION_MAJOR(driver.version);
    int minor = VERSION_MINOR(driver.version);
    int patch = VERSION_PATCH(driver.version);
    printf("%s: %d.%d.%d\n", driver.name, major, minor, patch);
}

static char *join_names(const char **names, size_t num)
{
    vector_t *vec = vector_of(num);

    for (size_t i = 0; i < num; i++)
    {
        vector_set(vec, i, (char *)names[i]);
    }

    return str_join(", ", vec);
}

static void print_help(const char **argv)
{
    printf("usage: %s <files... & objects... & options...>\n", argv[0]);

#define SECTION(id) printf("%s options:\n", id);
#define COMMAND(name, type, initial, description, ...)                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        const char *names[] = __VA_ARGS__;                                                                             \
        size_t total = sizeof(names) / sizeof(const char *);                                                           \
        const char *parts = join_names(names, total);                                                                  \
        printf("  %-20s : %s\n", parts, description);                                                                  \
    } while (0);

#include "flags.inc"
}

static void rename_module(hlir_t *hlir, const char *path)
{
    if (hlir->name != NULL)
    {
        return;
    }
    hlir->name = str_filename(path);
}

typedef struct
{
    file_t file;
    scan_t scanner;

    compile_t compileContext;
} context_t;

typedef enum
{
    OUTPUT_C89,
    OUTPUT_WASM,
    OUTPUT_TOTAL
} target_t;

static const map_pair_t kTargetNames[] = {
    STRING_CASE("c89", OUTPUT_C89),
    STRING_CASE("wasm", OUTPUT_WASM),
};

#define END_STAGE(name)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        status = end_reports(reports, name, &reportSettings);                                                          \
        if (status != 0)                                                                                               \
        {                                                                                                              \
            return status;                                                                                             \
        }                                                                                                              \
    } while (0)

#define FOR_EACH_SOURCE(idx, ctx, ...)                                                                                 \
    for (size_t idx = 0; idx < numSources; idx++)                                                                      \
    {                                                                                                                  \
        context_t *ctx = vector_get(contexts, idx);                                                                    \
        __VA_ARGS__                                                                                                    \
    }

#define DO_COMPILATION_STEP(function, name, ...)                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        FOR_EACH_SOURCE(i, ctx, {                                                                                      \
            driver.function(&runtime, &ctx->compileContext);                                                           \
            __VA_ARGS__                                                                                                \
        })                                                                                                             \
        END_STAGE(name);                                                                                               \
    } while (0)

static const char *get_output_name(target_t target, bool bytecode)
{
    if (bytecode)
    {
        return "mod.hlir";
    }

    switch (target)
    {
    case OUTPUT_C89:
        return "out.c";
        break;
    case OUTPUT_WASM:
        return "out.wasm";
        break;

    default:
        return NULL;
    }
}

int main(int argc, const char **argv)
{
    common_init();

    driver_t driver = get_driver();

    reports_t *reports = begin_reports();
    commands_t commands = {0};
    int status = parse_commandline(reports, &commands, argc, argv);
    if (status != 0)
    {
        return status;
    }

    end_report_settings_t reportSettings = {.limit = commands.warningLimit,
                                            .warningsAreErrors = commands.warningsAsErrors};

    verbose = commands.verboseLogging;
    logverbose("setup verbose logging");

    vector_t *files = commands.files;
    const char *targetName = commands.outputTarget;
    const char *outFile = commands.outputFile;

    target_t target = MATCH_CASE(targetName, kTargetNames);

    if (target >= OUTPUT_TOTAL)
    {
        report(reports, ERROR, NULL, "invalid output target: %s", targetName);
    }

    if (outFile == NULL)
    {
        outFile = get_output_name(target, commands.enableBytecode);
    }

    if (commands.printHelp)
    {
        print_help(argv);
        return 0;
    }

    if (commands.printVersion)
    {
        print_version(driver);
        return 0;
    }

    END_STAGE("commandline");

    vector_t *plugins = vector_new(0); // all plugins
    //vector_t *modules = vector_new(0); // all library bytecode modules
    vector_t *sources = vector_new(0); // all source files
    size_t pluginIdCounter = 0;

    for (size_t i = 0; i < vector_len(files); i++)
    {
        const char *path = vector_get(files, i);
        plugin_handle_t *handle = is_plugin(&pluginIdCounter, path);
        if (handle != NULL)
        {
            vector_push(&plugins, handle);
            continue;
        }

#if 0
        if (is_hlir_module(path))
        {
            vector_push(&modules, (char *)path);
            continue;
        }
#endif

        vector_push(&sources, (char *)path);
    }

    // size_t totalModules = vector_len(modules);
    runtime_t runtime = {
        .reports = reports,
        .modules = map_optimal(64),
    };

#if 0
    vector_t *loadedModules = vector_new(totalModules);

    for (size_t i = 0; i < totalModules; i++)
    {
        const char *path = vector_get(modules, i);
        vector_t *moduleItems = load_modules(reports, path);
        size_t numItems = vector_len(moduleItems);

        for (size_t j = 0; j < numItems; j++)
        {
            hlir_t *hlir = vector_get(moduleItems, j);
            vector_push(&loadedModules, hlir);
            map_set(runtime.modules, hlir->name, hlir);
        }
    }
#endif

    for (size_t i = 0; i < vector_len(plugins); i++)
    {
        plugin_handle_t *handle = vector_get(plugins, i);
        if (!plugin_load(reports, handle))
        {
            continue;
        }

        if (handle->init != NULL)
        {
            handle->init(&handle->plugin);
        }
    }

    size_t numSources = vector_len(sources);
    vector_t *contexts = vector_of(numSources);

    for (size_t i = 0; i < numSources; i++)
    {
        const char *path = vector_get(sources, i);

        context_t *ctx = ctu_malloc(sizeof(context_t));
        ctx->compileContext.ast = NULL;
        ctx->compileContext.hlirModule = NULL;

        error_t error = 0;
        file_t handle = file_open(path, 0, &error);

        if (error != 0)
        {
            message_t *id = report(reports, ERROR, NULL, "failed to open file `%s`", path);
            report_note(id, "%s", error_string(error));
            continue;
        }

        ctx->file = handle;
        vector_set(contexts, i, ctx);
    }

    END_STAGE("opening files");

    // create scanners for all our sources
    FOR_EACH_SOURCE(i, ctx, {
        ctx->scanner = scan_file(reports, driver.name, ctx->file);
        ctx->compileContext.scanner = &ctx->scanner;
    })

    END_STAGE("scanning files");

    // parse all source files
    DO_COMPILATION_STEP(fnParseFile, "parsing", {});

    // forward declare their contents
    DO_COMPILATION_STEP(fnForwardDecls, "forward declarations", {});

    // now rename all modules and add them to the import map
    FOR_EACH_SOURCE(i, ctx, {
        const char *path = vector_get(sources, i);
        hlir_t *hlirModule = ctx->compileContext.hlirModule;
        CTASSERTF(hlirModule != NULL, "module %s is null", path);

        rename_module(hlirModule, path);
        map_set(runtime.modules, get_hlir_name(hlirModule), hlirModule);
    })

    // now resolve all imports
    DO_COMPILATION_STEP(fnResolveImports, "resolving imports", {});

    // now compile all modules
    DO_COMPILATION_STEP(fnCompileModule, "compiling modules", {});

    FOR_EACH_SOURCE(i, ctx, { check_module(reports, ctx->compileContext.hlirModule); })

    END_STAGE("checking modules");

#if 0
    if (commands.enableBytecode)
    {
        // collect all modules together
        vector_t *bytecodeModules = vector_of(numSources);
        FOR_EACH_SOURCE(i, ctx, {
            hlir_t *hlirModule = ctx->compileContext.hlirModule;
            vector_set(bytecodeModules, i, hlirModule);
        })

        for (size_t i = 0; i < totalModules; i++)
        {
            hlir_t *hlir = vector_get(loadedModules, i);
            vector_push(&bytecodeModules, hlir);
        }

        // and save them to our intermediate format
        save_settings_t settings = {.embedSource = commands.embedSource};

        save_modules(reports, &settings, bytecodeModules, outFile);
        END_STAGE("module renaming");
        return status;
    }
#endif

    vector_t *allModules = vector_new(numSources/* + totalModules */);

    for (size_t i = 0; i < numSources; i++)
    {
        context_t *ctx = vector_get(contexts, i);
        hlir_t *hlirModule = ctx->compileContext.hlirModule;
        vector_push(&allModules, hlirModule);
    }

#if 0
    for (size_t i = 0; i < totalModules; i++)
    {
        const char *path = vector_get(modules, i);
        hlir_t *hlirModule = map_get(runtime.modules, path);
        vector_push(&allModules, hlirModule);
    }
#endif

    error_t error = 0;
    file_t out = file_open(outFile, FILE_WRITE | FILE_BINARY, &error);

    if (error != 0)
    {
        message_t *id = report(reports, ERROR, NULL, "failed to open file `%s`", outFile);
        report_note(id, "%s", error_string(error));
        return status;
    }

    c89_emit_modules(reports, allModules, out);

#if 0
    wasm_settings_t wasmSettings = {.defaultModule = commands.defaultWasmModule};

    switch (target)
    {
    case OUTPUT_C89:
        c89_emit_modules(reports, allModules, out);
        break;
    case OUTPUT_WASM:
        wasm_emit_modules(reports, allModules, out, wasmSettings);
        break;

    default:
        report(reports, ERROR, NULL, "unsupported output format");
    }
#endif

    file_close(out);

    logverbose("finished compiling %zu modules", vector_len(allModules));

    return end_reports(reports, "emitting code", &reportSettings);
}
