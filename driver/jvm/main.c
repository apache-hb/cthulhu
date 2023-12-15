#include "cthulhu/mediator/driver.h"

#include "core/macros.h"

static void jvm_create(driver_t *handle)
{
    CTU_UNUSED(handle);
}

static void jvm_destroy(driver_t *handle)
{
    CTU_UNUSED(handle);
}

static const char *kLangNames[] = { "class", "jar", NULL };

const language_t kJvmModule = {
    .id = "jvm",
    .name = "Java bytecode",
    .version = {
        .license = "LGPLv3",
        .desc = "Java bytecode interop driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(1, 0, 0)
    },

    .exts = kLangNames,

    .fnCreate = jvm_create,
    .fnDestroy = jvm_destroy
};
