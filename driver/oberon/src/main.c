#include "cthulhu/mediator/driver.h"

#include "memory/memory.h"
#include "oberon/driver.h"

#include "interop/compile.h"

#include "core/macros.h"

#include "obr_bison.h" // IWYU pragma: keep
#include "obr_flex.h" // IWYU pragma: keep

CTU_CALLBACKS(kCallbacks, obr);

static void obr_preparse(driver_t *handle, scan_t *scan)
{
    CTU_UNUSED(handle);
    lifetime_t *lifetime = handle_get_lifetime(handle);
    logger_t *reports = lifetime_get_logger(lifetime);

    obr_scan_t info = {
        .reports = reports,
    };

    scan_set_context(scan, ctu_memdup(&info, sizeof(obr_scan_t), get_global_arena()));
}

static void obr_postparse(driver_t *handle, scan_t *scan, void *tree)
{
    CTU_UNUSED(scan);

    vector_t *modules = tree;

    lifetime_t *lifetime = handle_get_lifetime(handle);

    size_t len = vector_len(modules);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *mod = vector_get(modules, i);
        context_t *ctx = context_new(handle, mod->name, mod, NULL);
        add_context(lifetime, vector_init(mod->name), ctx);
    }
}

static void obr_destroy(driver_t *handle) { CTU_UNUSED(handle); }

static const char *kLangNames[] = { "m", "mod", "obr", "oberon", NULL };

const language_t kOberonModule = {
    .id = "obr",
    .name = "Oberon-2",
    .version = {
        .license = "LGPLv3",
        .desc = "Oberon-2 language frontend",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(1, 0, 0)
    },

    .exts = kLangNames,

    .fn_create = obr_create,
    .fn_destroy = obr_destroy,

    .fn_prepass = obr_preparse,
    .fn_postpass = obr_postparse,
    .callbacks = &kCallbacks,

    .fn_compile_passes = {
        [eStageForwardSymbols] = obr_forward_decls,
        [eStageCompileImports] = obr_process_imports,
        [eStageCompileSymbols] = obr_compile_module
    }
};
