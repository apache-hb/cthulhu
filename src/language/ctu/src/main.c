#include "ctu/driver.h"

#include "cthulhu/broker/broker.h"

#include "ctu/sema/sema.h"
#include "interop/compile.h"

#include "std/vector.h"
#include "std/str.h"

#include "arena/arena.h"

#include "driver/driver.h"

#include "ctu_bison.h" // IWYU pragma: keep
#include "ctu_flex.h" // IWYU pragma: keep

CT_CALLBACKS(kCallbacks, ctu);

static vector_t *mod_basename(const char *fp, arena_t *arena)
{
    return vector_init(str_basename(fp, arena), arena);
}

static const vector_t *find_mod_path(ctu_t *ast, const char *fp, arena_t *arena)
{
    if (ast == NULL) { return mod_basename(fp, arena); }

    return vector_len(ast->modspec) > 0
        ? ast->modspec
        : mod_basename(fp, arena);
}

static void ctu_postparse(language_runtime_t *runtime, scan_t *scan, void *tree)
{
    ctu_t *ast = tree;
    CTASSERT(ast->kind == eCtuModule);

    arena_t *arena = runtime->arena;
    const vector_t *path = find_mod_path(ast, scan_path(scan), arena);

    size_t len = vector_len(ast->decls);
    size_t sizes[eCtuTagTotal] = {
        [eCtuTagValues] = len,
        [eCtuTagTypes] = len,
        [eCtuTagFunctions] = len,
        [eCtuTagModules] = len,
        [eCtuTagImports] = vector_len(ast->imports),
        [eCtuTagAttribs] = len,
        [eCtuTagSuffixes] = len,
    };

    lang_add_unit(runtime, build_unit_id(path, arena), ast->node, ast, sizes, eCtuTagTotal);
}

#define NEW_EVENT(name, ...) const diagnostic_t kEvent_##name = __VA_ARGS__;
#include "ctu/ctu.def"

static const diagnostic_t * const kDiagnosticTable[] = {
#define NEW_EVENT(name, ...) &kEvent_##name,
#include "ctu/ctu.def"
};

static const char *const kLangNames[] = { "ct", "ctu", "cthulhu", NULL };

static const size_t kDeclSizes[eCtuTagTotal] = {
    [eCtuTagValues] = 1,
    [eCtuTagTypes] = 1,
    [eCtuTagFunctions] = 1,
    [eCtuTagModules] = 1,
    [eCtuTagImports] = 1,
    [eCtuTagAttribs] = 1,
    [eCtuTagSuffixes] = 1,
};

static const char *const kDeclNames[eCtuTagTotal] = {
#define DECL_TAG(id, val, name) [id] = (name),
#include "ctu/ctu.def"
};

CT_DRIVER_API const language_t kCtuModule = {
    .info = {
        .id = "lang-cthulhu",
        .name = "Cthulhu",
        .version = {
            .license = "LGPLv3",
            .desc = "Cthulhu language driver",
            .author = "Elliot Haisley",
            .version = CT_NEW_VERSION(0, 4, 0),
        },

        .diagnostics = {
            .diagnostics = kDiagnosticTable,
            .count = sizeof(kDiagnosticTable) / sizeof(diagnostic_t*),
        },
    },

    .builtin = {
        .name = CT_TEXT_VIEW("ctu\0lang"),
        .decls = kDeclSizes,
        .names = kDeclNames,
        .length = eCtuTagTotal,
    },

    .exts = kLangNames,

    .ast_size = sizeof(ctu_t),

    .fn_create = ctu_init,

    .fn_postparse = ctu_postparse,
    .scanner = &kCallbacks,

    .fn_passes = {
        [ePassForwardDecls] = ctu_forward_decls,
        [ePassImportModules] = ctu_process_imports
    }
};

CT_LANG_EXPORT(kCtuModule)
