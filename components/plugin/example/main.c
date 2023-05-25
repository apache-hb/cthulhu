#include "cthulhu/mediator/plugin.h"

#include <stdio.h>

static void plugin_init(plugin_handle_t *handle)
{
    printf("plugin_init\n");
}

static const plugin_t kPluginInfo = {
    .id = "example",
    .name = "example",
    .version = {
        .license = "MIT",
        .desc = "Example plugin for the Cthulhu Compiler Collection",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(1, 1, 0)
    },

    .fnInit = plugin_init,
};

extern const plugin_t *example_acquire(mediator_t *mediator)
{
    return &kPluginInfo;
}
