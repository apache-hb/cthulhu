#include "config/config.h"
#include "core/macros.h"

#include "memory/arena.h"
#include "pl0/sema.h"
#include "pl0/ast.h"

#include "cthulhu/runtime/driver.h"

#include "interop/compile.h"

#include "std/str.h"

#include "pl0_bison.h" // IWYU pragma: keep
#include "pl0_flex.h" // IWYU pragma: keep

CTU_CALLBACKS(kCallbacks, pl0);

static void *pl0_preparse(driver_t *handle, scan_t *scan)
{
    CTU_UNUSED(scan);

    lifetime_t *lifetime = handle_get_lifetime(handle);
    logger_t *reports = lifetime_get_logger(lifetime);
    arena_t *arena = lifetime_get_arena(lifetime);

    pl0_scan_t info = {
        .arena = arena,
        .reports = reports
    };

    return arena_memdup(&info, sizeof(pl0_scan_t), arena);
}

static void pl0_postparse(driver_t *handle, scan_t *scan, void *tree)
{
    pl0_t *ast = tree;
    CTASSERT(ast->type == ePl0Module);

    // TODO: dedup this with pl0_forward_decls
    const char *fp = scan_path(scan);
    vector_t *path = vector_len(ast->mod) > 0
        ? ast->mod
        : vector_init(str_filename_noext(fp));

    lifetime_t *lifetime = handle_get_lifetime(handle);
    context_t *ctx = context_new(handle, vector_tail(path), ast, NULL);

    add_context(lifetime, path, ctx);
}

static const char *const kLangNames[] = { "pl", "pl0", NULL };

const language_t kPl0Module = {
    .id = "pl0",
    .name = "PL/0",
    .version = {
        .license = "LGPLv3",
        .desc = "PL/0 language driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(2, 3, 2)
    },

    .exts = kLangNames,

    .fn_create = pl0_init,

    .fn_preparse = pl0_preparse,
    .fn_postparse = pl0_postparse,
    .parse_callbacks = &kCallbacks,

    .fn_compile_passes = {
        [eStageForwardSymbols] = pl0_forward_decls,
        [eStageCompileImports] = pl0_process_imports,
        [eStageCompileSymbols] = pl0_compile_module
    }
};
