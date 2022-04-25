#include "cthulhu/plugins/plugin.h"

PLUGIN_EXPORT const plugin_info_t kPluginInfo = {
    .name = "hello",
    .version = "1.0.0",
    .description = "example plugin",
    .license = "GPLv3",
};

PLUGIN_EXPORT void ctPluginInit(plugin_t *info) {
    report(info->reports, NOTE, NULL, "hello from a plugin");
}
