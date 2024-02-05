#include "common.h"

#include "core/macros.h"
#include "os/os.h"

#include "base/panic.h"

#if CTU_LOADER_DYNAMIC

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
        lang_main_t lang = (lang_main_t)os_library_symbol(&library, CT_LANG_ENTRY);
        if (lang != NULL)
        {
            mod.type |= eModLanguage;
            mod.lang = lang();
            mod.error = eLoadErrorNone;
        }
    }

    if (mask & eModTarget)
    {
        target_main_t target = (target_main_t)os_library_symbol(&library, CT_TARGET_ENTRY);
        if (target != NULL)
        {
            mod.type |= eModTarget;
            mod.target = target();
            mod.error = eLoadErrorNone;
        }
    }

    if (mask & eModPlugin)
    {
        plugin_main_t plugin = (plugin_main_t)os_library_symbol(&library, CT_PLUGIN_ENTRY);
        if (plugin != NULL)
        {
            mod.type |= eModPlugin;
            mod.plugin = plugin();
            mod.error = eLoadErrorNone;
        }
    }

    return mod;
}

#else

loaded_module_t load_shared_module(loader_t *loader, module_type_t mask, const char *name)
{
    CT_UNUSED(loader);
    CT_UNUSED(mask);
    CT_UNUSED(name);

    return load_error(eLoadErrorDisabled, 0);
}

#endif
