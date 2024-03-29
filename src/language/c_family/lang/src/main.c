// SPDX-License-Identifier: GPL-3.0-only

#include "driver/driver.h"

#include "c/sema/sema.h"
#include "c/driver.h"

#include "cthulhu/broker/broker.h"

#include "interop/compile.h"

#include "core/macros.h"

#include "cc_bison.h" // IWYU pragma: keep
#include "cc_flex.h" // IWYU pragma: keep

CT_CALLBACKS(kCallbacks, cc);

static void cc_postparse(language_runtime_t *runtime, scan_t *scan, void *tree)
{
    CT_UNUSED(runtime);
    CT_UNUSED(scan);
    CT_UNUSED(tree);
}

static const diagnostic_t * const kDiagnosticTable[] = {
#define NEW_EVENT(name, ...) &kEvent_##name,
#include "c/events.inc"
};

static const char *const kLangNames[] = CT_LANG_EXTS("c", "h");

static const size_t kDeclSizes[eCTagTotal] = {
    [eCTagValues] = 1,
    [eCTagTypes] = 1,
    [eCTagProcs] = 1,
    [eCTagModules] = 1,
};

CT_DRIVER_API const language_t kCModule = {
    .info = {
        .id = "lang/c",
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

    .ast_size = sizeof(c_ast_t),

    .fn_create = cc_create,
    .fn_destroy = cc_destroy,

    .fn_postparse = cc_postparse,
    .scanner = &kCallbacks,
};

CT_LANG_EXPORT(kCModule)
