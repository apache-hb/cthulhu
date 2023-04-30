#include "cthulhu/mediator/mediator.h"

#include "base/memory.h"
#include "base/panic.h"

#include "std/map.h"
#include "std/vector.h"

#include "platform/file.h"
#include "stacktrace/stacktrace.h"

#include "cthulhu/hlir/init.h"

typedef struct mediator_t
{
    const char *name; ///< mediator name
    version_t version; ///< mediator version

    map_t *extMap; ///< map of extensions to their language

    map_t *instances; ///< map of languages to their instance

    vector_t *plugins; ///< list of plugins
} mediator_t;

static lang_handle_t *add_new_lang_instance(mediator_t *self, const language_t *lang)
{
    lang_handle_t *instance = ctu_malloc(sizeof(lang_handle_t));
    instance->handle = lang;
    instance->user = NULL;
    instance->mediator = self;

    map_set_ptr(self->instances, lang, instance);

    return instance;
}

static plugin_handle_t *add_new_plugin_instance(mediator_t *self, const plugin_t *plugin)
{
    plugin_handle_t *instance = ctu_malloc(sizeof(plugin_handle_t));
    instance->handle = plugin;
    instance->user = NULL;
    instance->mediator = self;

    return instance;
}

static void add_lang_ext(mediator_t *self, const char *ext, const language_t *lang)
{
    map_set(self->extMap, ext, (language_t*)lang);
}

// public api

void runtime_init()
{
    GLOBAL_INIT();

    stacktrace_init();
    platform_init();
    init_gmp(&globalAlloc);
    init_hlir();
}

mediator_t *mediator_new(const char *name, version_t version)
{
    mediator_t *mediator = ctu_malloc(sizeof(mediator_t));
    mediator->name = name;
    mediator->version = version;
    
    mediator->extMap = map_new(64);
    mediator->instances = map_new(64);
    mediator->plugins = vector_new(64);

    return mediator;
}

void mediator_add_language(mediator_t *self, const language_t *language, ap_t *ap)
{
    CTASSERT(self != NULL);
    CTASSERT(language != NULL);

    size_t idx = 0;
    while (language->exts[idx] != NULL)
    {
        add_lang_ext(self, language->exts[idx], language);
        idx += 1;
    }

    lang_handle_t *handle = add_new_lang_instance(self, language);

    if (language->fnConfigure != NULL)
        language->fnConfigure(handle, ap);
}

void mediator_add_plugin(mediator_t *self, const plugin_t *plugin, ap_t *ap)
{
    CTASSERT(self != NULL);
    CTASSERT(plugin != NULL);

    plugin_handle_t *handle = add_new_plugin_instance(self, plugin);

    if (plugin->fnConfigure != NULL)
        plugin->fnConfigure(handle, ap);

    vector_push(&self->plugins, handle);
}

void mediator_region(mediator_t *self, region_t region)
{
    size_t len = vector_len(self->plugins);
    for (size_t i = 0; i < len; i++)
    {
        plugin_handle_t *handle = vector_get(self->plugins, i);
        const plugin_t *plugin = handle->handle;
        if (plugin->fnRegion == NULL)
            continue;
            
        plugin->fnRegion(handle, region);
    }
}

void mediator_startup(mediator_t *self)
{
    // init all plugins
    size_t len = vector_len(self->plugins);
    for (size_t i = 0; i < len; i++)
    {
        plugin_handle_t *handle = vector_get(self->plugins, i);
        const plugin_t *plugin = handle->handle;
        if (plugin->fnInit == NULL)
            continue;
            
        plugin->fnInit(handle);
    }

    // init all languages
    map_iter_t iter = map_iter(self->instances);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        lang_handle_t *handle = entry.value;
        const language_t *lang = handle->handle;
        if (lang->fnInit == NULL)
            continue;

        lang->fnInit(handle);
    }
}

void mediator_shutdown(mediator_t *self)
{
    size_t len = vector_len(self->plugins);
    for (size_t i = 0; i < len; i++)
    {
        plugin_handle_t *handle = vector_get(self->plugins, i);
        const plugin_t *plugin = handle->handle;
        if (plugin->fnShutdown == NULL)
            continue;
            
        plugin->fnShutdown(handle);
    }
}
