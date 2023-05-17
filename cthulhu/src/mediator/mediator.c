#include "cthulhu/mediator/mediator.h"
#include "cthulhu/mediator/language.h"
#include "cthulhu/mediator/plugin.h"

#include "base/memory.h"
#include "base/panic.h"

#include "std/map.h"
#include "std/vector.h"

#include "platform/file.h"
#include "stacktrace/stacktrace.h"

#include "scan/scan.h"

#include "report/report.h"

#include "cthulhu/hlir/init.h"

#include "common.h"

typedef struct mediator_t
{
    const char *name; ///< mediator name
    version_t version; ///< mediator version

    map_t *langById; /// map_t<const char *, const language_t *>
    map_t *langByExt; /// map_t<const char *, const language_t *>

    map_t *pluginById; /// map_t<const char *, const plugin_t *>

    vector_t *languages;
    vector_t *plugins;
} mediator_t;

// public mediator api

void runtime_init(void)
{
    GLOBAL_INIT();

    stacktrace_init();
    platform_init();
    init_gmp(&globalAlloc);
    init_hlir();
}

// mediator api

mediator_t *mediator_new(const char *name, version_t version)
{
    CTASSERT(name != NULL);

    mediator_t *self = ctu_malloc(sizeof(mediator_t));

    self->name = name;
    self->version = version;

    self->langById = map_optimal(64);
    self->langByExt = map_optimal(64);

    self->pluginById = map_optimal(64);

    self->languages = vector_new(4);
    self->plugins = vector_new(4);

    return self;
}

void mediator_load_language(mediator_t *self, const language_t *language)
{
    CTASSERT(self != NULL);
    CTASSERT(language != NULL);

    CTASSERTF(language->id != NULL, "language has no id");
    CTASSERTF(language->name != NULL, "language '%s' has no name", language->id);
    CTASSERTF(*language->exts != NULL, "language '%s' has no extensions", language->id);

    CTASSERTF(language->fnParse != NULL, "language '%s' has no parse function", language->id);

    size_t idx = 0;
    while (language->exts[idx] != NULL)
    {
        const language_t *old = mediator_register_extension(self, language->exts[idx++], language);

        // TODO: gracefully handle this
        CTASSERTF(old == NULL, "%p", old);
    }

    map_set(self->langById, language->id, (void*)language);

    vector_push(&self->languages, (void*)language);

    EXEC(language, fnLoad, self);
}

void mediator_load_plugin(mediator_t *self, const plugin_t *plugin)
{
    CTASSERT(self != NULL);
    CTASSERT(plugin != NULL);

    CTASSERTF(plugin->id != NULL, "plugin has no id");
    CTASSERTF(plugin->name != NULL, "plugin '%s' has no name", plugin->id);

    const plugin_t *old = map_get(self->pluginById, plugin->id);
    CTASSERT(old == NULL); // TODO: gracefully handle this

    map_set(self->pluginById, plugin->id, (void*)plugin);

    vector_push(&self->plugins, (void*)plugin);

    EXEC(plugin, fnLoad, self);
}

void mediator_unload_language(mediator_t *self, const language_t *language)
{
    EXEC(language, fnUnload, self);
}

void mediator_unload_plugin(mediator_t *self, const plugin_t *plugin)
{
    EXEC(plugin, fnUnload, self);
}

const language_t *mediator_register_extension(mediator_t *self, const char *ext, const language_t *lang)
{
    CTASSERT(self != NULL);
    CTASSERT(ext != NULL);
    CTASSERT(lang != NULL);

    const language_t *old = map_get(self->langByExt, ext);
    if (old != NULL) { return old; }

    map_set(self->langByExt, ext, (void*)lang);
    return NULL;
}

const language_t *mediator_get_language(mediator_t *self, const char *id)
{
    CTASSERT(self != NULL);
    CTASSERT(id != NULL);

    return map_get(self->langById, id);
}

const language_t *mediator_get_language_by_ext(mediator_t *self, const char *ext)
{
    CTASSERT(self != NULL);
    CTASSERT(ext != NULL);

    return map_get(self->langByExt, ext);
}
