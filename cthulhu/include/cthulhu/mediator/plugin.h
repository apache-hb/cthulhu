#pragma once

#include "mediator.h"

// plugin api

typedef void (*plugin_load_t)(mediator_t *);
typedef void (*plugin_unload_t)(mediator_t *);

typedef void (*plugin_configure_t)(plugin_handle_t *, ap_t *);

typedef void (*plugin_init_t)(plugin_handle_t *);
typedef void (*plugin_deinit_t)(plugin_handle_t *);

typedef struct plugin_t
{
    const char *id; ///< language driver id
    const char *name; ///< language driver name

    version_info_t version; ///< language driver version

    plugin_load_t fnLoad;
    plugin_unload_t fnUnload;

    plugin_configure_t fnConfigure; ///< configure the mediator to work with this driver

    plugin_init_t fnInit; ///< initialize the language driver
    plugin_deinit_t fnDeinit; ///< shutdown the language driver
} plugin_t;

typedef const plugin_t *(*plugin_acquire_t)(mediator_t *);

#define PLUGIN_ENTRY_POINT plugin_acquire

#ifdef CC_MSVC
#   define PLUGIN_EXPORT __declspec(dllexport)
#else
#   define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

// handle api

void plugin_set_user(plugin_handle_t *self, void *user);
void *plugin_get_user(plugin_handle_t *self);

plugin_handle_t *lifetime_get_plugin_handle(lifetime_t *self, const plugin_t *it);
