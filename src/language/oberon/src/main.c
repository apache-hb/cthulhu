#include "cthulhu/runtime/driver.h"

#include "arena/arena.h"
#include "oberon/driver.h"

#include "interop/compile.h"

#include "core/macros.h"

#include "driver/driver.h"

#include "obr_bison.h" // IWYU pragma: keep
#include "obr_flex.h" // IWYU pragma: keep

CTU_CALLBACKS(kCallbacks, obr);

static void *obr_preparse(driver_t *handle, scan_t *scan)
{
    CTU_UNUSED(scan);

    lifetime_t *lifetime = handle_get_lifetime(handle);
    arena_t *arena = lifetime_get_arena(lifetime);
    logger_t *reports = lifetime_get_logger(lifetime);

    obr_scan_t info = {
        .reports = reports,
    };

    return arena_memdup(&info, sizeof(obr_scan_t), arena);
}

static void obr_postparse(driver_t *handle, scan_t *scan, void *tree)
{
    CTU_UNUSED(scan);

    vector_t *modules = tree;

    lifetime_t *lifetime = handle_get_lifetime(handle);
    arena_t *arena = lifetime_get_arena(lifetime);

    size_t len = vector_len(modules);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *mod = vector_get(modules, i);
        context_t *ctx = context_new(handle, mod->name, mod, NULL);
        add_context(lifetime, vector_init(mod->name, arena), ctx);
    }
}

static void obr_destroy(driver_t *handle) { CTU_UNUSED(handle); }

#define NEW_EVENT(id, ...) const diagnostic_t kEvent_##id = __VA_ARGS__;
#include "oberon/events.def"

static const diagnostic_t *const kDiagnosticTable[] = {
#define NEW_EVENT(id, ...) &kEvent_##id,
#include "oberon/events.def"
};

static const char *const kLangNames[] = { "m", "mod", "obr", "oberon", NULL };

CT_DRIVER_API const language_t kOberonModule = {
    .id = "obr",
    .name = "Oberon-2",
    .version = {
        .license = "LGPLv3",
        .desc = "Oberon-2 language frontend",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(1, 0, 0)
    },

    .exts = kLangNames,

    .diagnostics = {
        .diagnostics = kDiagnosticTable,
        .count = sizeof(kDiagnosticTable) / sizeof(const diagnostic_t *)
    },

    .fn_create = obr_create,
    .fn_destroy = obr_destroy,

    .fn_preparse = obr_preparse,
    .fn_postparse = obr_postparse,
    .parse_callbacks = &kCallbacks,

    .fn_compile_passes = {
        [eStageForwardSymbols] = obr_forward_decls,
        [eStageCompileImports] = obr_process_imports,
        [eStageCompileSymbols] = obr_compile_module
    }
};

CTU_DRIVER_ENTRY(kOberonModule)
