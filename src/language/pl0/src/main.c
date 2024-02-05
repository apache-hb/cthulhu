#include "config/config.h"

#include "arena/arena.h"
#include "pl0/sema.h"
#include "pl0/ast.h"

#include "cthulhu/broker/broker.h"

#include "interop/compile.h"

#include "std/str.h"

#include "driver/driver.h"

#include "pl0_bison.h" // IWYU pragma: keep
#include "pl0_flex.h" // IWYU pragma: keep

CTU_CALLBACKS(kCallbacks, pl0);

static void pl0_preparse(language_runtime_t *runtime, void *context)
{
    pl0_scan_t *info = context;
    info->logger = runtime->logger;
}

static void pl0_postparse(language_runtime_t *runtime, scan_t *scan, void *tree)
{
    pl0_t *ast = tree;
    CTASSERT(ast->type == ePl0Module);
    arena_t *arena = runtime->arena;

    // TODO: dedup this with pl0_forward_decls
    const char *fp = scan_path(scan);
    vector_t *path = vector_len(ast->mod) > 0
        ? ast->mod
        : vector_init(str_basename(fp, arena), arena);

    compile_unit_t *unit = lang_new_unit(runtime, vector_tail(path), ast);

    lang_add_unit(runtime, build_unit_id(path, arena), unit);
}

static const char *const kLangNames[] = { "pl", "pl0", NULL };

static const size_t kRootSizes[ePl0TagTotal] = {
    [ePl0TagValues] = 1,
    [ePl0TagTypes] = 1,
    [ePl0TagProcs] = 1,
    [ePl0TagModules] = 1
};

CT_DRIVER_API const language_t kPl0Module = {
    .info = {
        .id = "pl0",
        .name = "PL/0",
        .version = {
            .license = "LGPLv3",
            .author = "Elliot Haisley",
            .desc = "PL/0 language driver",
            .version = CT_NEW_VERSION(2, 3, 2)
        },
    },

    .builtin = {
        .name = CT_TEXT_VIEW("pl0\0lang"),
        .decls = kRootSizes,
        .length = ePl0TagTotal,
    },

    .context_size = sizeof(pl0_scan_t),

    .exts = kLangNames,

    .fn_create = pl0_init,

    .fn_preparse = pl0_preparse,
    .fn_postparse = pl0_postparse,
    .scanner = &kCallbacks,

    .fn_passes = {
        [ePassForwardDecls] = pl0_forward_decls,
        [ePassImportModules] = pl0_process_imports,
        [ePassCompileDecls] = pl0_compile_module
    }
};

CT_LANG_EXPORT(kPl0Module)
