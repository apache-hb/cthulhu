#include "cthulhu/runtime/runtime.h"

#include "driver/driver.h"

const language_t kCppModule = {
    .id = "cpp",
    .name = "C Preprocessor",
    .version = {
        .license = "LGPLv3",
        .desc = "C Preprocessor language driver",
        .author = "Elliot Haisley",
        .version = CT_NEW_VERSION(0, 0, 1)
    },
};

CTU_DRIVER_ENTRY(kCppModule)
