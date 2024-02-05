#include "support/support.h"

#include "arena/arena.h"
#include "base/panic.h"

#include "cthulhu/broker/broker.h"

#include "cthulhu/events/events.h"
#include "notify/notify.h"
#include "std/typed/vector.h"
#include "std/map.h"
#include "std/set.h"

typedef struct support_t
{
    arena_t *arena;
    broker_t *broker;
    loader_t *loader;

    set_t *langs;

    // map of extensions to the associated language runtime
    // map_t<const char*, language_runtime_t*>
    map_t *extmap;
} support_t;

static void add_loaded_module(support_t *support, loaded_module_t mod)
{
    logger_t *logger = broker_get_logger(support->broker);
    const node_t *node = broker_get_node(support->broker);
    if (mod.type & eModLanguage)
    {
        const language_t *lang = mod.lang;
        const module_info_t *info = &lang->info;
        if (set_contains(support->langs, info->id))
        {
            msg_notify(logger, &kEvent_LanguageDriverConflict, node, "language %s already loaded", info->id);
        }

        set_add(support->langs, info->id);

        language_runtime_t *runtime = broker_add_language(support->broker, lang);

        for (size_t i = 0; lang->exts[i]; i++)
        {
            const char *ext = lang->exts[i];
            const language_runtime_t *old = map_get(support->extmap, ext);
            if (old != NULL)
            {
                const language_t *ol = old->info;
                const module_info_t *prev = &ol->info;
                msg_notify(logger, &kEvent_ExtensionConflict, node, "extension %s already associated with language %s", ext, prev->id);
            }
            map_set(support->extmap, ext, runtime);
        }
    }
}

support_t *support_new(broker_t *broker, loader_t *loader, arena_t *arena)
{
    CTASSERT(broker != NULL);
    CTASSERT(loader != NULL);

    support_t *support = ARENA_MALLOC(sizeof(support_t), "support", NULL, arena);
    support->arena = arena;
    support->broker = broker;
    support->loader = loader;

    support->langs = set_new(16, kTypeInfoString, arena);
    support->extmap = map_new(16, kTypeInfoString, arena);

    return support;
}

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

bool support_load_module(support_t *support, module_type_t mask, const char *name)
{
    CTASSERT(support != NULL);
    CTASSERT(name != NULL);

    loaded_module_t mod = load_module(support->loader, mask, name);
    add_loaded_module(support, mod);

    return mod.type != eModNone;
}

language_runtime_t *support_get_lang(support_t *support, const char *ext)
{
    CTASSERT(support != NULL);

    return map_get(support->extmap, ext);
}
