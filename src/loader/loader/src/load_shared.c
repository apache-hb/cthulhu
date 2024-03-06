// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

#include "driver/driver.h"
#include "os/os.h"

#include "base/panic.h"

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

    loaded_module_t mod = { .type = eModNone, .error = eLoadErrorNoEntry };

    if (mask & eModLanguage)
    {
        lang_main_t fn_lang;
        error = os_library_symbol(&library, (os_symbol_t*)&fn_lang, CT_LANG_ENTRY);
        if (error == 0)
        {
            mod.type |= eModLanguage;
            mod.lang = fn_lang();
            mod.error = eLoadErrorNone;
        }
    }

    if (mask & eModTarget)
    {
        target_main_t fn_target;
        error = os_library_symbol(&library, (os_symbol_t*)&fn_target, CT_TARGET_ENTRY);
        if (error == 0)
        {
            mod.type |= eModTarget;
            mod.target = fn_target();
            mod.error = eLoadErrorNone;
        }
    }

    if (mask & eModPlugin)
    {
        plugin_main_t fn_plugin;
        error = os_library_symbol(&library, (os_symbol_t*)&fn_plugin, CT_PLUGIN_ENTRY);
        if (error == 0)
        {
            mod.type |= eModPlugin;
            mod.plugin = fn_plugin();
            mod.error = eLoadErrorNone;
        }
    }

    // TODO: what should we do if this fails?
    (void)os_library_close(&library);

    return mod;
}
