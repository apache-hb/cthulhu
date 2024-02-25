// SPDX-License-Identifier: GPL-3.0-only

#include "debug/target.h"

#include "driver/driver.h"

CT_DRIVER_API const target_t kTargetDebug = {
    .info = {
        .id = "target/debug",
        .name = "Debug",
        .version = {
            .license = "LGPLv3",
            .author = "Elliot Haisley",
            .desc = "Debug code generation target",
            .version = CT_NEW_VERSION(0, 0, 1),
        },
    },

    .fn_ssa = debug_ssa
};

CT_TARGET_EXPORT(kTargetDebug)
