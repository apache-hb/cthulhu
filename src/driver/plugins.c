#include "plugins.h"
#include "cthulhu/util/util.h"
#include "src/driver/plugins.h"

#define PLUGIN_INFO_NAME "kPluginInfo"
#define PLUGIN_INIT_NAME "ctPluginInit"

plugin_handle_t *is_plugin(size_t *id, const char *filename)
{
    error_t error = 0;
    library_t handle = library_open(filename, &error);
    if (error != 0 || handle == NULL)
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
    error_t error = 0;
    plugin_info_t *info = library_get(handle->libraryHandle, PLUGIN_INFO_NAME, &error);

    if (error != 0 || info == NULL)
    {
        report(reports, WARNING, NULL, "failed to load plugin %s: missing plugin info", handle->path);
        return false;
    }

    logverbose("loaded plugin %s", info->name);
    logverbose(" version: %d.%d.%d", VERSION_MAJOR(info->version), VERSION_MINOR(info->version), VERSION_PATCH(info->version));
    logverbose(" description: %s", info->description);
    logverbose(" license: %s", info->license);

    handle->info = info;
    handle->init = library_get(handle->libraryHandle, PLUGIN_INIT_NAME, &error);

    handle->plugin.reports = reports;

    return handle;
}
