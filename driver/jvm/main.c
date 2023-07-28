#include "cthulhu/mediator/driver.h"

#include "base/macros.h"

static void jvm_config(lifetime_t *lifetime, ap_t *ap)
{
    CTU_UNUSED(lifetime);
    CTU_UNUSED(ap);
}

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

    .fnConfig = jvm_config,

    .fnCreate = jvm_create,
    .fnDestroy = jvm_destroy
};
