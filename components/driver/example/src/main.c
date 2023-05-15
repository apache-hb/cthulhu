#include "cthulhu/mediator/language.h"
#include "cthulhu/mediator/mediator.h"

#include <stdio.h>

static const char *kLangNames[] = { ".e", ".example", NULL };

static void ex_load(mediator_t *mediator)
{
    printf("ex-load(%p)\n", mediator);
}

static void ex_unload(mediator_t *mediator)
{
    printf("ex-unload(%p)\n", mediator);
}

static void ex_init(lang_handle_t *handle)
{
    printf("ex-init(%p)\n", handle);
}

static void ex_deinit(lang_handle_t *handle)
{
    printf("ex-deinit(%p)\n", handle);
}

static const language_t kLanguageInfo = {
    .id = "example",
    .name = "Example",
    .version = {
        .license = "GPLv3",
        .desc = "Example language driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(1, 0, 0)
    },

    .exts = kLangNames,
    
    .fnLoad = ex_load,
    .fnUnload = ex_unload,

    .fnInit = ex_init,
    .fnDeinit = ex_deinit,
};

LANGUAGE_EXPORT
extern const language_t *LANGUAGE_ENTRY_POINT(mediator_t *mediator)
{
    return &kLanguageInfo;
}
