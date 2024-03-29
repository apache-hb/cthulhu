// SPDX-License-Identifier: GPL-3.0-only

#include "cthulhu/runtime/driver.h"

#include "driver/driver.h"

#include "core/macros.h"

static void jvm_create(driver_t *handle)
{
    CT_UNUSED(handle);
}

static void jvm_destroy(driver_t *handle)
{
    CT_UNUSED(handle);
}

static const char *const kLangNames[] = CT_LANG_EXTS("class", "jar");

const language_t kJvmModule = {
    .id = "lang/jvm",
    .name = "Java bytecode",
    .version = {
        .license = "LGPLv3",
        .desc = "Java bytecode interop driver",
        .author = "Elliot Haisley",
        .version = CT_NEW_VERSION(1, 0, 0)
    },

    .exts = kLangNames,

    .fn_create = jvm_create,
    .fn_destroy = jvm_destroy
};

CTU_DRIVER_ENTRY(kJvmModule)
