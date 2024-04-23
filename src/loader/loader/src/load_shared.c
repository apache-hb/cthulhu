// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

#include "driver/driver.h"
#include "os/os.h"

#include "base/panic.h"
#include <stdio.h>

STA_DECL
loaded_module_t load_shared_module(loader_t *loader, module_type_t mask, const char *name)
{
    CTASSERT(loader != NULL);
    CTASSERT(name != NULL);

    os_library_t library = { 0 };
    os_error_t error = os_library_open(name, &library);

    if (error)
    {
        return load_error(eLoadErrorLibrary, error);
    }

    loaded_module_t mod = { .type = eModNone, .library = library, .error = eLoadErrorNoEntry };

    if (mask & eModLanguage)
    {
        lang_main_t fn_lang;
        error = os_library_symbol(&library, (void**)&fn_lang, CT_LANG_ENTRY);
        if (error == eOsSuccess)
        {
            mod.type |= eModLanguage;
            mod.lang = fn_lang();
            mod.error = eLoadErrorNone;
        }
    }

    if (mask & eModTarget)
    {
        target_main_t fn_target;
        error = os_library_symbol(&library, (void**)&fn_target, CT_TARGET_ENTRY);
        if (error == eOsSuccess)
        {
            mod.type |= eModTarget;
            mod.target = fn_target();
            mod.error = eLoadErrorNone;
        }
    }

    if (mask & eModPlugin)
    {
        plugin_main_t fn_plugin;
        error = os_library_symbol(&library, (void**)&fn_plugin, CT_PLUGIN_ENTRY);
        if (error == eOsSuccess)
        {
            mod.type |= eModPlugin;
            mod.plugin = fn_plugin();
            mod.error = eLoadErrorNone;
        }
    }

    return mod;
}
