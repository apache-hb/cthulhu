#include "driver/driver.h"

#include "c/sema/sema.h"
#include "c/driver.h"

#include "cthulhu/broker/broker.h"

#include "interop/compile.h"

#include "core/macros.h"

#include "cc_bison.h" // IWYU pragma: keep
#include "cc_flex.h" // IWYU pragma: keep

CT_CALLBACKS(kCallbacks, cc);

static void cc_preparse(language_runtime_t *runtime, void *context)
{
    cc_scan_t *ctx = context;
    ctx->logger = runtime->logger;
}

static void cc_postparse(language_runtime_t *runtime, scan_t *scan, void *tree)
{
    CT_UNUSED(runtime);
    CT_UNUSED(scan);
    CT_UNUSED(tree);
}

static const diagnostic_t * const kDiagnosticTable[] = {
#define NEW_EVENT(name, ...) &kEvent_##name,
#include "c/events.def"
};

static const char *const kLangNames[] = { "c", "h", NULL };

static const size_t kDeclSizes[eCTagTotal] = {
    [eCTagValues] = 1,
    [eCTagTypes] = 1,
    [eCTagProcs] = 1,
    [eCTagModules] = 1,
};

CT_DRIVER_API const language_t kCModule = {
    .info = {
        .id = "c",
        .name = "C",
        .version = {
            .license = "LGPLv3",
            .desc = "C language driver",
            .author = "Elliot Haisley",
            .version = CT_NEW_VERSION(0, 0, 1),
        },

        .diagnostics = {
            .diagnostics = kDiagnosticTable,
            .count = sizeof(kDiagnosticTable) / sizeof(diagnostic_t*),
        },
    },

    .builtin = {
        .name = CT_TEXT_VIEW("cc\0lang"),
        .decls = kDeclSizes,
        .length = eCTagTotal,
    },

    .exts = kLangNames,

    .context_size = sizeof(cc_scan_t),
    .ast_size = sizeof(c_ast_t),

    .fn_create = cc_create,
    .fn_destroy = cc_destroy,

    .fn_preparse = cc_preparse,
    .fn_postparse = cc_postparse,
    .scanner = &kCallbacks,
};

CT_LANG_EXPORT(kCModule)
