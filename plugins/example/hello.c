#include "cthulhu/plugins/plugin.h"

PLUGIN_INFO("hello", NEW_VERSION(1, 0, 1), "example plugin", "GPLv3");

PLUGIN_EXPORT void ctPluginInit(plugin_t *info)
{
    report(info->reports, NOTE, NULL, "hello from a plugin");
}
