#include "cthulhu/mediator/driver.h"

static const char *kLangNames[] = { "class", "jar", NULL };

const language_t kPl0Module = {
    .id = "jvm",
    .name = "Java bytecode",
    .version = {
        .license = "LGPLv3",
        .desc = "Java bytecode interop driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(1, 0, 0)
    },

    .exts = kLangNames
};
