#include "plugins.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/util.h"
#include "src/driver/plugins.h"

#include <dlfcn.h>

#define PLUGIN_INFO_NAME "kPluginInfo"
#define PLUGIN_INIT_NAME "ctPluginInit"

plugin_handle_t *is_plugin(size_t *id, const char *filename) {
    void *handle = dlopen(filename, RTLD_LAZY);
    if (handle == NULL) {
        return NULL;
    }

    plugin_handle_t *plugin = ctu_malloc(sizeof(plugin_handle_t));
    plugin->handle = handle;
    plugin->path = filename;
    plugin->plugin.id = *id++;
    return plugin;
}

bool plugin_load(reports_t *reports, plugin_handle_t *handle) {
    plugin_info_t *info = dlsym(handle->handle, PLUGIN_INFO_NAME);
    if (info == NULL) {
        report(
            reports,
            WARNING,
            NULL,
            "failed to load plugin %s: missing plugin info",
            handle->path);
        return NULL;
    }

    logverbose("loaded plugin %s", info->name);
    logverbose(" version: %s", info->version);
    logverbose(" description: %s", info->description);
    logverbose(" license: %s", info->license);

    handle->info = info;
    handle->init = dlsym(handle->handle, PLUGIN_INIT_NAME);

    handle->plugin.reports = reports;

    return handle;
}
