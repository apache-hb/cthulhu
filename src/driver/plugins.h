#pragma once

#include "cthulhu/plugins/plugin.h"
#include "cthulhu/util/library.h"
#include "cthulhu/util/report.h"

typedef void (*plugin_init_t)(plugin_t *);

typedef struct
{
    library_t libraryHandle;
    const char *path;

    plugin_info_t *info;
    plugin_init_t init;
    plugin_t plugin;
} plugin_handle_t;

plugin_handle_t *is_plugin(size_t *id, const char *filename);

bool plugin_load(reports_t *reports, plugin_handle_t *handle);