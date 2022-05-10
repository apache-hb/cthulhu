#include "cthulhu/interface/plugin.h"

PLUGIN_EXPORT void ctPluginInit(plugin_t *info)
{
    report(info->reports, NOTE, NULL, "hello from a plugin");
}
