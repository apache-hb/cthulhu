#include "plugins.h"
#include "src/platform/platform.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/util.h"
#include "src/driver/plugins.h"

#define PLUGIN_INFO_NAME "kPluginInfo"
#define PLUGIN_INIT_NAME "ctPluginInit"

plugin_handle_t *is_plugin(size_t *id, const char *filename)
{
    error_t error = ERROR_SUCCESS;
    library_handle_t handle = native_library_open(filename, &error);
    if (error != ERROR_SUCCESS)
    {
        return NULL;
    }

    plugin_handle_t *plugin = ctu_malloc(sizeof(plugin_handle_t));
    plugin->libraryHandle = handle;
    plugin->path = filename;
    plugin->plugin.id = *id++;
    return plugin;
}

bool plugin_load(reports_t *reports, plugin_handle_t *handle)
{
    error_t error = ERROR_SUCCESS;
    plugin_info_t *info = native_library_get_symbol(handle->libraryHandle, PLUGIN_INFO_NAME, &error);
    
    if (error != ERROR_SUCCESS)
    {
        report(reports, WARNING, NULL, "failed to load plugin %s: missing plugin info", handle->path);
        return false;
    }

    logverbose("loaded plugin %s", info->name);
    logverbose(" version: %s", info->version);
    logverbose(" description: %s", info->description);
    logverbose(" license: %s", info->license);

    handle->info = info;
    handle->init = native_library_get_symbol(handle->libraryHandle, PLUGIN_INIT_NAME, &error);

    handle->plugin.reports = reports;

    return handle;
}
