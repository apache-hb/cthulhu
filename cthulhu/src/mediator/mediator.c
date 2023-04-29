#include "cthulhu/mediator/mediator.h"

#include "base/memory.h"
#include "base/panic.h"

#include "std/map.h"

#include "platform/file.h"
#include "stacktrace/stacktrace.h"

#include "cthulhu/hlir/init.h"

typedef struct mediator_t
{
    const char *name; ///< mediator name
    version_t version; ///< mediator version

    map_t *extMap; ///< map of extensions to their language

    map_t *instances; ///< map of languages to their instance
} mediator_t;

typedef struct instance_t
{
    mediator_t *mediator;
} instance_t;

static instance_t *add_new_instance(mediator_t *self, const language_t *lang)
{
    instance_t *instance = ctu_malloc(sizeof(instance_t));
    instance->mediator = self;

    map_set_ptr(self->instances, lang, instance);

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

    return mediator;
}

void mediator_add_language(mediator_t *self, const language_t *language)
{
    CTASSERT(self != NULL);
    CTASSERT(language != NULL);

    size_t idx = 0;
    while (language->exts[idx] != NULL)
    {
        add_lang_ext(self, language->exts[idx], language);
        idx += 1;
    }

    instance_t *handle = add_new_instance(self, language);

    language->fnConfigure(handle);
}

void mediator_add_plugin(mediator_t *self, const plugin_t *plugin)
{
    CTASSERT(self != NULL);
    CTASSERT(plugin != NULL);

    plugin->fnConfigure(self);
}
