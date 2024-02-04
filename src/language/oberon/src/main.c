#include "cthulhu/broker/broker.h"

#include "arena/arena.h"
#include "oberon/driver.h"

#include "interop/compile.h"

#include "core/macros.h"

#include "driver/driver.h"

#include "oberon/sema/sema.h"
#include "obr_bison.h" // IWYU pragma: keep
#include "obr_flex.h" // IWYU pragma: keep

CTU_CALLBACKS(kCallbacks, obr);

static void *obr_preparse(language_runtime_t *runtime)
{
    arena_t *arena = lang_get_arena(runtime);
    logger_t *logger = lang_get_logger(runtime);

    obr_scan_t info = {
        .logger = logger,
    };

    return arena_memdup(&info, sizeof(obr_scan_t), arena);
}

static void obr_postparse(language_runtime_t *runtime, scan_t *scan, void *tree)
{
    vector_t *modules = tree;

    arena_t *arena = lang_get_arena(runtime);

    size_t len = vector_len(modules);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *mod = vector_get(modules, i);
        compile_unit_t *unit = lang_new_unit(runtime, mod->name, mod, NULL);
        lang_add_unit(runtime, vector_init(mod->name, arena), unit);
    }
}

static void obr_destroy(language_runtime_t *handle) { CT_UNUSED(handle); }

#define NEW_EVENT(id, ...) const diagnostic_t kEvent_##id = __VA_ARGS__;
#include "oberon/events.def"

static const diagnostic_t *const kDiagnosticTable[] = {
#define NEW_EVENT(id, ...) &kEvent_##id,
#include "oberon/events.def"
};

static const char *const kLangNames[] = { "m", "mod", "obr", "oberon", NULL };

static const size_t kDeclSizes[eObrTagTotal] = {
    [eObrTagValues] = 32,
    [eObrTagTypes] = 32,
    [eObrTagProcs] = 32,
    [eObrTagModules] = 32,
};

CT_DRIVER_API const language_t kOberonModule = {
    .info = {
        .id = "obr",
        .name = "Oberon-2",
        .version = {
            .license = "LGPLv3",
            .desc = "Oberon-2 language frontend",
            .author = "Elliot Haisley",
            .version = CT_NEW_VERSION(1, 0, 0)
        },

        .diagnostics = {
            .diagnostics = kDiagnosticTable,
            .count = sizeof(kDiagnosticTable) / sizeof(const diagnostic_t *)
        },
    },

    .builtin = {
        .name = CT_TEXT_VIEW("obr\0lang"),
        .decls = kDeclSizes,
        .length = eObrTagTotal,
    },

    .exts = kLangNames,
    .fn_create = obr_create,
    .fn_destroy = obr_destroy,

    .fn_preparse = obr_preparse,
    .fn_postparse = obr_postparse,
    .scanner = &kCallbacks,

    .fn_passes = {
        [eStageForwardSymbols] = obr_forward_decls,
        [eStageCompileImports] = obr_process_imports,
        [eStageCompileSymbols] = obr_compile_module
    }
};

CTU_DRIVER_ENTRY(kOberonModule)
