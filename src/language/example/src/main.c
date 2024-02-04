#include "cthulhu/runtime/driver.h"

#include "cthulhu/tree/tree.h"

#include "memory/memory.h"
#include "std/vector.h"

#include "scan/node.h"

#include "base/log.h"

#include "driver/driver.h"

static vector_t *example_lang_path(arena_t *arena)
{
    vector_t *path = vector_new(2, arena);
    vector_push(&path, "example");
    vector_push(&path, "lang");
    return path;
}

static tree_t *example_lang_module(driver_t *driver)
{
    lifetime_t *lifetime = handle_get_lifetime(driver);
    logger_t *reports = lifetime_get_logger(lifetime);
    tree_cookie_t *cookie = lifetime_get_cookie(lifetime);
    arena_t *arena = lifetime_get_arena(lifetime);

    const node_t *node = handle_get_builtin(driver);
    size_t sizes[eSemaTotal] = {
        [eSemaValues] = 1,
        [eSemaTypes] = 1,
        [eSemaProcs] = 1,
        [eSemaModules] = 1
    };

    return tree_module_root(reports, cookie, node, "runtime", eSemaTotal, sizes, arena);
}

static void ex_create(driver_t *handle)
{
    lifetime_t *lifetime = handle_get_lifetime(handle);
    arena_t *arena = lifetime_get_arena(lifetime);

    vector_t *path = example_lang_path(arena);
    tree_t *mod = example_lang_module(handle);
    context_t *ctx = compiled_new(handle, mod);

    add_context(lifetime, path, ctx);

    ctu_log("ex_create(handle = 0x%p)", (void*)handle);
}

static void ex_destroy(driver_t *handle)
{
    ctu_log("ex_destroy(handle = 0x%p)", (void*)handle);
}

static void ex_parse(driver_t *handle, scan_t *scan)
{
    ctu_log("ex_parse(handle = 0x%p, scan = %s)", (void*)handle, scan_path(scan));
}

static void ex_forward_symbols(context_t *context)
{
    ctu_log("ex_forward(context = 0x%p)", (void*)context);
}

static void ex_compile_imports(context_t *context)
{
    ctu_log("ex_compile_imports(context = 0x%p)", (void*)context);
}

static void ex_compile_types(context_t *context)
{
    ctu_log("ex_compile_types(context = 0x%p)", (void*)context);
}

static void ex_compile_symbols(context_t *context)
{
    ctu_log("ex_compile_symbols(context = 0x%p)", (void*)context);
}

static const char * const kLangNames[] = { "e", "example", NULL };

CT_DRIVER_API const language_t kExampleModule = {
    .info = {
        .id = "example",
        .name = "Example",
        .version = {
            .license = "GPLv3",
            .desc = "Example language driver",
            .author = "Elliot Haisley",
            .version = CT_NEW_VERSION(1, 0, 2)
        },
    },

    .exts = kLangNames,

    .fn_create = ex_create,
    .fn_destroy = ex_destroy,

    .fn_parse = ex_parse,
    .fn_compile_passes = {
        [eStageForwardSymbols] = ex_forward_symbols,
        [eStageCompileImports] = ex_compile_imports,
        [eStageCompileTypes] = ex_compile_types,
        [eStageCompileSymbols] = ex_compile_symbols
    }
};

CTU_DRIVER_ENTRY(kExampleModule)
