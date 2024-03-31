// SPDX-License-Identifier: LGPL-3.0-only

#include "support/support.h"

#include "arena/arena.h"
#include "base/panic.h"

#include "cthulhu/broker/broker.h"

#include "cthulhu/events/events.h"
#include "notify/notify.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/map.h"
#include "std/set.h"

typedef struct support_t
{
    arena_t *arena;
    broker_t *broker;
    loader_t *loader;

    typevec_t *modules;

    map_t *languages;

    // map of extensions to the associated language runtime
    // map_t<const char*, language_runtime_t*>
    map_t *extmap;

    // map_t<const char*, plugin_runtime_t*>
    map_t *plugins;

    // map_t<const char*, target_runtime_t*>
    map_t *targets;
} support_t;

static void add_loaded_module(support_t *support, loaded_module_t mod)
{
    typevec_push(support->modules, &mod);

    logger_t *logger = broker_get_logger(support->broker);
    const node_t *node = broker_get_node(support->broker);
    if (mod.type & eModLanguage)
    {
        const language_t *lang = mod.lang;
        const module_info_t *info = &lang->info;
        CTASSERTF_ALWAYS(str_startswith(info->id, "lang/"), "language id `%s` must start with `lang/`", info->id);

        const char *id = info->id + sizeof("lang/") - 1;

        if (map_contains(support->languages, id))
        {
            msg_notify(logger, &kEvent_LanguageDriverConflict, node, "%s already loaded", info->id);
        }

        language_runtime_t *runtime = broker_add_language(support->broker, lang);
        map_set(support->languages, id, runtime);

        for (size_t i = 0; lang->exts[i]; i++)
        {
            const char *ext = lang->exts[i];
            const language_runtime_t *old = map_get(support->extmap, ext);
            if (old != NULL)
            {
                const language_t *ol = old->info;
                const module_info_t *prev = &ol->info;
                msg_notify(logger, &kEvent_ExtensionConflict, node, "extension %s already associated with %s", ext, prev->id);
            }
            map_set(support->extmap, ext, runtime);
        }
    }

    if (mod.type & eModPlugin)
    {
        const plugin_t *plugin = mod.plugin;
        const module_info_t *info = &plugin->info;
        CTASSERTF_ALWAYS(str_startswith(info->id, "plugin/"), "plugin id `%s` must start with `plugin/`", info->id);

        const char *id = info->id + sizeof("plugin/") - 1;

        if (map_contains(support->plugins, id))
        {
            msg_notify(logger, &kEvent_PluginConflict, node, "%s already loaded", info->id);
        }

        plugin_runtime_t *runtime = broker_add_plugin(support->broker, plugin);
        map_set(support->plugins, id, runtime);
    }

    if (mod.type & eModTarget)
    {
        const target_t *target = mod.target;
        const module_info_t *info = &target->info;
        CTASSERTF_ALWAYS(str_startswith(info->id, "target/"), "target id `%s` must start with `target/`", info->id);

        const char *id = info->id + sizeof("target/") - 1;

        if (map_contains(support->targets, id))
        {
            msg_notify(logger, &kEvent_TargetConflict, node, "%s already loaded", info->id);
        }

        target_runtime_t *runtime = broker_add_target(support->broker, target);
        map_set(support->targets, id, runtime);
    }
}

USE_DECL
support_t *support_new(broker_t *broker, loader_t *loader, arena_t *arena)
{
    CTASSERT(broker != NULL);
    CTASSERT(loader != NULL);

    support_t *support = ARENA_MALLOC(sizeof(support_t), "support", NULL, arena);
    support->arena = arena;
    support->broker = broker;
    support->loader = loader;

    support->modules = typevec_new(sizeof(loaded_module_t), 16, arena);
    support->languages = map_new(16, kTypeInfoString, arena);
    support->extmap = map_new(16, kTypeInfoString, arena);
    support->plugins = map_new(16, kTypeInfoString, arena);
    support->targets = map_new(16, kTypeInfoString, arena);

    ARENA_IDENTIFY(support->modules, "modules", support, arena);
    ARENA_IDENTIFY(support->languages, "languages", support, arena);
    ARENA_IDENTIFY(support->extmap, "extmap", support, arena);
    ARENA_IDENTIFY(support->plugins, "plugins", support, arena);
    ARENA_IDENTIFY(support->targets, "targets", support, arena);

    return support;
}

USE_DECL
void support_load_default_modules(support_t *support)
{
    CTASSERT(support != NULL);

    typevec_t *mods = load_default_modules(support->loader);
    size_t len = typevec_len(mods);
    for (size_t i = 0; i < len; i++)
    {
        loaded_module_t *mod = typevec_offset(mods, i);
        add_loaded_module(support, *mod);
    }
}

USE_DECL
bool support_load_module(support_t *support, module_type_t mask, const char *name, loaded_module_t *out)
{
    CTASSERT(support != NULL);
    CTASSERT(name != NULL);
    CTASSERT(out != NULL);

    loaded_module_t mod = load_module(support->loader, mask, name);
    add_loaded_module(support, mod);

    *out = mod;

    return mod.type != eModNone;
}

USE_DECL
typevec_t *support_get_modules(support_t *support)
{
    CTASSERT(support != NULL);

    return support->modules;
}

USE_DECL
language_runtime_t *support_get_lang(support_t *support, const char *ext)
{
    CTASSERT(support != NULL);

    return map_get(support->extmap, ext);
}

USE_DECL
plugin_runtime_t *support_get_plugin(support_t *support, const char *name)
{
    CTASSERT(support != NULL);

    return map_get(support->extmap, name);
}

USE_DECL
target_runtime_t *support_get_target(support_t *support, const char *name)
{
    CTASSERT(support != NULL);

    return map_get(support->extmap, name);
}
