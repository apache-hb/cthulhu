#include "cthulhu/plugins/plugin.h"

PLUGIN_INFO("hello", "1.0.0", "example plugin", "GPLv3");

PLUGIN_EXPORT void ctPluginInit(plugin_t *info)
{
    report(info->reports, NOTE, NULL, "hello from a plugin");
}
