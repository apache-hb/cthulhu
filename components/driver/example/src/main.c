#include "cthulhu/mediator/mediator2.h"

static const char *kLangNames[] = { ".e", ".example", NULL };

static const language_t kLanguageInfo = {
    .id = "example",
    .name = "Example",
    .version = {
        .license = "GPLv3",
        .desc = "Example language driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(1, 0, 0)
    },

    .exts = kLangNames
};

LANGUAGE_EXPORT
extern const language_t *LANGUAGE_ENTRY_POINT(mediator_t *mediator)
{
    return &kLanguageInfo;
}
