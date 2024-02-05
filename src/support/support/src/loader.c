#include "common.h"

#include "arena/arena.h"
#include "base/panic.h"
#include "std/typed/vector.h"

static const loader_config_t kConfig = eLoadNone
#if CTU_LOADER_STATIC
    | eLoadStatic
#endif
#if CTU_LOADER_DYNAMIC
    | eLoadDynamic
#endif
;

typedef struct loader_t
{
    arena_t *arena;
} loader_t;

loaded_module_t load_error(load_error_t error, os_error_t os)
{
    loaded_module_t mod = {
        .type = eModNone,
        .error = error,
        .os = os
    };
    return mod;
}

USE_DECL
loader_config_t loader_config(void)
{
    return kConfig;
}

USE_DECL
loader_t *loader_new(arena_t *arena)
{
    CTASSERT(arena != NULL);

    loader_t *loader = ARENA_MALLOC(sizeof(loader_t), "loader", NULL, arena);
    loader->arena = arena;

    return loader;
}

USE_DECL
typevec_t *load_default_modules(loader_t *loader)
{
    CTASSERT(loader != NULL);

#if CTU_LOADER_STATIC
    static_modules_t mods = get_static_modules();

    typevec_t *vec = typevec_new(sizeof(loaded_module_t), CT_LANG_COUNT + CT_PLUGIN_COUNT + CT_TARGET_COUNT, loader->arena);

    for (size_t i = 0; i < CT_LANG_COUNT; i++)
    {
        const language_t *lang = mods.langs[i];
        loaded_module_t mod = {
            .type = eModLanguage,
            .lang = lang
        };
        typevec_push(vec, &mod);
    }

    for (size_t i = 0; i < CT_PLUGIN_COUNT; i++)
    {
        const plugin_t *plugin = mods.plugins[i];
        loaded_module_t mod = {
            .type = eModPlugin,
            .plugin = plugin
        };
        typevec_push(vec, &mod);
    }

    for (size_t i = 0; i < CT_TARGET_COUNT; i++)
    {
        const target_t *target = mods.targets[i];
        loaded_module_t mod = {
            .type = eModTarget,
            .target = target
        };
        typevec_push(vec, &mod);
    }

    return vec;
#else
    return typevec_new(sizeof(loaded_module_t), 0, loader->arena);
#endif
}

USE_DECL
loaded_module_t load_module(loader_t *loader, module_type_t mask, const char *name)
{
    CTASSERT(loader != NULL);
    CTASSERT(name != NULL);

    loaded_module_t static_mod = load_static_module(loader, mask, name);
    loaded_module_t shared_mod = load_shared_module(loader, mask, name);

    loaded_module_t result = { .type = static_mod.type | shared_mod.type };

    // select the shared modules if they are available
    result.lang = shared_mod.lang ? shared_mod.lang : static_mod.lang;
    result.plugin = shared_mod.plugin ? shared_mod.plugin : static_mod.plugin;
    result.target = shared_mod.target ? shared_mod.target : static_mod.target;

    return result;
}
