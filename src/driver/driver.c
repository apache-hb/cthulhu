#include "cthulhu/util/error.h"

#include "cmd.h"
#include "cthulhu/driver/driver.h"
#include "cthulhu/hlir/init.h"
#include "cthulhu/util/io.h"
#include "cthulhu/util/version-def.h"
#include "plugins.h"

#include "cthulhu/ast/compile.h"
#include "cthulhu/emit/emit.h"
#include "cthulhu/hlir/query.h"
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
    hlir->name = ctu_filename(path);
}

void common_init(void)
{
    init_gmp();
    init_hlir();
}

typedef struct
{
    file_t *file;
    scan_t scanner;

    compile_t compileContext;
} context_t;

typedef enum
{
    OUTPUT_C89,
    OUTPUT_WASM
} target_t;

#if 0
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
#endif

#define END_STAGE(name)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        status = end_reports(reports, limit, name);                                                                    \
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

int common_main(int argc, const char **argv, driver_t driver)
{
    reports_t *reports = begin_reports();
    commands_t commands = {0};
    int status = parse_commandline(reports, &commands, argc, argv);
    if (status != 0)
    {
        return status;
    }

    verbose = commands.verboseLogging;

    vector_t *files = commands.files;
    size_t limit = commands.warningLimit;
    // const char *target = commands.outputTarget;
    const char *outFile = commands.outputFile;

    if (outFile == NULL)
    {
        outFile = commands.enableBytecode ? "mod.hlir" : "out.c";
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

    vector_t *plugins = vector_new(0); // all plugins
    vector_t *modules = vector_new(0); // all library bytecode modules
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

        if (is_hlir_module(path))
        {
            vector_push(&modules, (char *)path);
            continue;
        }

        vector_push(&sources, (char *)path);
    }

    size_t totalModules = vector_len(modules);
    runtime_t runtime = {
        .reports = reports,
        .modules = optimal_map(totalModules),
    };

    for (size_t i = 0; i < totalModules; i++)
    {
        const char *path = vector_get(modules, i);
        vector_t *moduleItems = load_modules(reports, path);
        size_t numItems = vector_len(moduleItems);

        for (size_t j = 0; j < numItems; j++)
        {
            hlir_t *hlir = vector_get(moduleItems, j);
            map_set(runtime.modules, hlir->name, hlir);
        }
    }

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

        file_t *file = file_new(path, TEXT, READ);
        if (!file_ok(file))
        {
            ctu_errno_t err = ctu_last_error();
            report(reports, ERROR, NULL, "failed to open file: %s", ctu_err_string(err));
            continue;
        }

        ctx->file = file;
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

    if (commands.enableBytecode)
    {
        // collect all modules together
        vector_t *bytecodeModules = vector_of(numSources);
        FOR_EACH_SOURCE(i, ctx, {
            hlir_t *hlirModule = ctx->compileContext.hlirModule;
            vector_push(&bytecodeModules, hlirModule);
        })

        // and save them to our intermediate format
        save_settings_t settings = {.embedSource = commands.embedSource};

        save_modules(reports, &settings, bytecodeModules, outFile);
        END_STAGE("module renaming");
        return status;
    }

    vector_t *allModules = vector_new(numSources + totalModules);

    for (size_t i = 0; i < numSources; i++)
    {
        context_t *ctx = vector_get(contexts, i);
        hlir_t *hlirModule = ctx->compileContext.hlirModule;
        vector_push(&allModules, hlirModule);
    }

    for (size_t i = 0; i < totalModules; i++)
    {
        const char *path = vector_get(modules, i);
        hlir_t *hlirModule = map_get(runtime.modules, path);
        vector_push(&allModules, hlirModule);
    }

    file_t *out = file_new(outFile, TEXT, WRITE);

    if (!file_ok(out))
    {
        ctu_errno_t err = ctu_last_error();
        report(reports, ERROR, NULL, "failed to open file: %s", ctu_err_string(err));
        return status;
    }

    c89_emit_modules(reports, allModules, out);
    close_file(out);

    return end_reports(reports, limit, "emitting code");
}

hlir_t *find_module(runtime_t *runtime, const char *path)
{
    return map_get(runtime->modules, path);
}
