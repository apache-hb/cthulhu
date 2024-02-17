#include "oberon/driver.h"

#include "oberon/sema/sema.h"

#include "cthulhu/broker/broker.h"

#include "driver/driver.h"

#include "interop/compile.h"

#include "std/vector.h"

#include "base/util.h"
#include "core/macros.h"

#include "obr_bison.h" // IWYU pragma: keep
#include "obr_flex.h" // IWYU pragma: keep

CT_CALLBACKS(kCallbacks, obr);

static void obr_postparse(language_runtime_t *runtime, scan_t *scan, void *tree)
{
    CT_UNUSED(scan);

    vector_t *modules = tree;

    size_t len = vector_len(modules);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *mod = vector_get(modules, i);

        size_t decl_count = vector_len(mod->decls);
        size_t sizes[eObrTagTotal] = {
            [eObrTagValues] = decl_count,
            [eObrTagTypes] = decl_count,
            [eObrTagProcs] = decl_count,
            [eObrTagModules] = 32,
        };

        lang_add_unit(runtime, text_view_from(mod->name), mod->node, mod, sizes, eObrTagTotal);
    }
}

static void obr_destroy(language_runtime_t *handle) { CT_UNUSED(handle); }

#define NEW_EVENT(id, ...) const diagnostic_t kEvent_##id = __VA_ARGS__;
#include "oberon/oberon.def"

static const diagnostic_t *const kDiagnosticTable[] = {
#define NEW_EVENT(id, ...) &kEvent_##id,
#include "oberon/oberon.def"
};

static const char *const kLangNames[] = { "m", "mod", "obr", "oberon", NULL };

static const size_t kDeclSizes[eObrTagTotal] = {
    [eObrTagValues] = 32,
    [eObrTagTypes] = 32,
    [eObrTagProcs] = 32,
    [eObrTagModules] = 32,
};

static const char *const kDeclNames[eObrTagTotal] = {
#define DECL_TAG(id, val, name) [id] = (name),
#include "oberon/oberon.def"
};

CT_DRIVER_API const language_t kOberonModule = {
    .info = {
        .id = "lang-oberon2",
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
        .names = kDeclNames,
        .length = eObrTagTotal,
    },

    .exts = kLangNames,

    .ast_size = sizeof(obr_t),

    .fn_create = obr_create,
    .fn_destroy = obr_destroy,

    .fn_postparse = obr_postparse,

    .scanner = &kCallbacks,

    .fn_passes = {
        [ePassForwardDecls] = obr_forward_decls,
        [ePassImportModules] = obr_process_imports
    }
};

CT_LANG_EXPORT(kOberonModule)
