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

CTU_CALLBACKS(kCallbacks, ctu);

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

static void *ctu_preparse(language_runtime_t *runtime)
{
    arena_t *arena = lang_get_arena(runtime);
    logger_t *logger = lang_get_logger(runtime);

    ctu_scan_t info = {
        .logger = logger,
        .attribs = vector_new(4, arena)
    };

    return arena_memdup(&info, sizeof(ctu_scan_t), arena);
}

static void ctu_postparse(language_runtime_t *runtime, scan_t *scan, void *tree)
{
    ctu_t *ast = tree;
    CTASSERT(ast->kind == eCtuModule);
    arena_t *arena = scan_get_arena(scan);

    const vector_t *path = find_mod_path(ast, scan_path(scan), arena);

    compile_unit_t *ctx = lang_new_unit(runtime, vector_tail(path), ast, NULL);

    lang_add_unit(runtime, path, ctx);
}

static const diagnostic_t * const kDiagnosticTable[] = {
#define NEW_EVENT(name, ...) &kEvent_##name,
#include "ctu/events.def"
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

CT_DRIVER_API const language_t kCtuModule = {
    .info = {
        .id = "ctu",
        .name = "Cthulhu",
        .version = {
            .license = "LGPLv3",
            .desc = "Cthulhu language driver",
            .author = "Elliot Haisley",
            .version = CT_NEW_VERSION(0, 4, 0)
        },

        .diagnostics = {
            .diagnostics = kDiagnosticTable,
            .count = sizeof(kDiagnosticTable) / sizeof(diagnostic_t*),
        },
    },

    .builtin = {
        .name = CT_TEXT_VIEW("ctulhu\0lang"),
        .decls = kDeclSizes,
        .length = eCtuTagTotal,
    },

    .exts = kLangNames,

    .fn_create = ctu_init,

    .fn_preparse = ctu_preparse,
    .fn_postparse = ctu_postparse,
    .scanner = &kCallbacks,

    .fn_passes = {
        [eStageForwardSymbols] = ctu_forward_decls,
        [eStageCompileImports] = ctu_process_imports,
        [eStageCompileSymbols] = ctu_compile_module
    }
};

CTU_DRIVER_ENTRY(kCtuModule)
