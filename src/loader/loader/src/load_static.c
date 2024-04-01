// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h" // IWYU pragma: export

#include <enum_modules.h>

#include "base/util.h"
#include "base/panic.h"

// TODO: find a way to nicely dedup this logic

USE_DECL
loaded_module_t load_static_module(loader_t *loader, module_type_t mask, const char *name)
{
    CTASSERT(loader != NULL);
    CTASSERT(name != NULL);

    static_modules_t mods = get_static_modules();

    loaded_module_t mod = { .type = eModNone };

    if (mask & eModLanguage)
    {
        for (size_t i = 0; i < CT_LANG_COUNT; i++)
        {
            const language_t *lang = mods.langs[i];
            const module_info_t *info = &lang->info;

            if (!str_equal(info->id, name))
                continue;

            if (mod.lang != NULL)
            {
                const language_t *old = mod.lang;
                const module_info_t *prev = &old->info;
                CT_NEVER("multiple static languages with the same id: %s (prev: %s, new: %s)", info->id, prev->name, info->name);
            }

            mod.type |= eModLanguage;
            mod.lang = lang;
        }
    }

    if (mask & eModPlugin)
    {
        for (size_t i = 0; i < CT_PLUGIN_COUNT; i++)
        {
            const plugin_t *plugin = mods.plugins[i];
            const module_info_t *info = &plugin->info;

            if (!str_equal(info->id, name))
                continue;

            if (mod.plugin != NULL)
            {
                const plugin_t *old = mod.plugin;
                const module_info_t *prev = &old->info;
                CT_NEVER("multiple static plugins with the same id: %s (prev: %s, new: %s)", info->id, prev->name, info->name);
            }

            mod.type |= eModPlugin;
            mod.plugin = plugin;
        }
    }

    if (mask & eModTarget)
    {
        for (size_t i = 0; i < CT_TARGET_COUNT; i++)
        {
            const target_t *target = mods.targets[i];
            const module_info_t *info = &target->info;
            if (!str_equal(info->id, name))
                continue;

            if (mod.target != NULL)
            {
                const target_t *old = mod.target;
                const module_info_t *prev = &old->info;
                CT_NEVER("multiple static targets with the same id: %s (prev: %s, new: %s)", info->id, prev->name, info->name);
            }

            mod.type |= eModTarget;
            mod.target = target;
        }
    }

    return mod;
}
