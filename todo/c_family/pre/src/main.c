// SPDX-License-Identifier: GPL-3.0-only

#include "cthulhu/broker/broker.h"

#include "driver/driver.h"

const language_t kCppModule = {
    .info = {
        .id = "lang/cpp",
        .name = "C Preprocessor",
        .version = {
            .license = "LGPLv3",
            .desc = "C Preprocessor language driver",
            .author = "Elliot Haisley",
            .version = CT_NEW_VERSION(0, 0, 1)
        },
    },
};

CT_LANG_EXPORT(kCppModule)
