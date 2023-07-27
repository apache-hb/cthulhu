#include "cthulhu/mediator/driver.h"

#include "base/macros.h"

static void obr_config(lifetime_t *lifetime, ap_t *ap)
{
    UNUSED(lifetime);
    UNUSED(ap);
}

static void obr_create(driver_t *handle)
{
    UNUSED(handle);
}

static void obr_destroy(driver_t *handle)
{
    UNUSED(handle);
}

static const char *kLangNames[] = { "m", "obr", NULL };

const language_t kOberonModule = {
    .id = "obr",
    .name = "Oberon-2",
    .version = {
        .license = "LGPLv3",
        .desc = "Oberon-2 language frontend",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(1, 0, 0)
    },

    .exts = kLangNames,

    .fnConfig = obr_config,

    .fnCreate = obr_create,
    .fnDestroy = obr_destroy
};
