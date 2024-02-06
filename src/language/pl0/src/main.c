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
    info->arena = runtime->arena;
    info->string_arena = runtime->arena;
    info->ast_arena = &runtime->ast_arena;
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

    size_t const_count = vector_len(ast->consts);
    size_t global_count = vector_len(ast->globals);
    size_t proc_count = vector_len(ast->procs);

    size_t sizes[ePl0TagTotal] = {
        [ePl0TagValues] = const_count + global_count,
        [ePl0TagProcs] = proc_count,
        [ePl0TagImportedValues] = 64,
        [ePl0TagImportedProcs] = 64
    };

    lang_add_unit(runtime, build_unit_id(path, arena), ast->node, ast, sizes, ePl0TagTotal);
}

static const char *const kLangNames[] = { "pl", "pl0", NULL };

static const size_t kRootSizes[ePl0TagTotal] = {
    [ePl0TagValues] = 1,
    [ePl0TagTypes] = 1,
    [ePl0TagProcs] = 1,
    [ePl0TagModules] = 1
};

static const char *const kDeclNames[ePl0TagTotal] = {
#define DECL_TAG(id, init, str) [id] = (str),
#include "pl0/pl0.def"
};

CT_DRIVER_API const language_t kPl0Module = {
    .info = {
        .id = "lang-pl0",
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
        .names = kDeclNames,
        .length = ePl0TagTotal,
    },

    .context_size = sizeof(pl0_scan_t),
    .ast_size = sizeof(pl0_t),

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
