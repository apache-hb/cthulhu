// SPDX-License-Identifier: GPL-3.0-only

#include "cthulhu/broker/broker.h"

#include "cthulhu/tree/tree.h"

#include "base/log.h"

#include "driver/driver.h"

static void ex_create(language_runtime_t *runtime, tree_t *root)
{
    ctu_log("ex_create(runtime = 0x%p, root = 0x%p)", (void*)runtime, (void*)root);
}

static void ex_destroy(language_runtime_t *handle)
{
    ctu_log("ex_destroy(handle = 0x%p)", (void*)handle);
}

static void ex_forward_symbols(language_runtime_t *context, compile_unit_t *unit)
{
    ctu_log("ex_forward(context = 0x%p, unit = 0x%p)", (void*)context, (void*)unit);
}

static void ex_compile_imports(language_runtime_t *context, compile_unit_t *unit)
{
    ctu_log("ex_compile_imports(context = 0x%p, unit = 0x%p)", (void*)context, (void*)unit);
}

static void ex_compile_types(language_runtime_t *context, compile_unit_t *unit)
{
    ctu_log("ex_compile_types(context = 0x%p, unit = 0x%p)", (void*)context, (void*)unit);
}

static void ex_compile_symbols(language_runtime_t *context, compile_unit_t *unit)
{
    ctu_log("ex_compile_symbols(context = 0x%p, unit = 0x%p)", (void*)context, (void*)unit);
}

static const char * const kLangNames[] = CT_LANG_EXTS("e", "example");

static const size_t kDeclSizes[eSemaCount] = {
    [eSemaValues] = 1,
    [eSemaTypes] = 1,
    [eSemaProcs] = 1,
    [eSemaModules] = 1,
};

CT_DRIVER_API const language_t kExampleModule = {
    .info = {
        .id = "lang/example",
        .name = "Example",
        .version = {
            .license = "GPLv3",
            .desc = "Example language driver",
            .author = "Elliot Haisley",
            .version = CT_NEW_VERSION(1, 0, 2)
        },
    },

    .builtin = {
        .name = CT_TEXT_VIEW("example\0lang"),
        .decls = kDeclSizes,
        .length = eSemaCount,
    },

    .exts = kLangNames,

    .fn_create = ex_create,
    .fn_destroy = ex_destroy,

    .fn_passes = {
        [ePassForwardDecls] = ex_forward_symbols,
        [ePassImportModules] = ex_compile_imports,
        [ePassCompileTypes] = ex_compile_types,
        [ePassCompileDecls] = ex_compile_symbols
    }
};

CT_LANG_EXPORT(kExampleModule)
