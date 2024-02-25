// SPDX-License-Identifier: GPL-3.0-only

#include "hlsl/target.h"

#include "driver/driver.h"

CT_DRIVER_API const target_t kTargetHlsl = {
    .info = {
        .id = "target/hlsl",
        .name = "HLSL",
        .version = {
            .license = "LGPLv3",
            .author = "Elliot Haisley",
            .desc = "HLSL code generation target",
            .version = CT_NEW_VERSION(0, 0, 1),
        },
    },
};

CT_TARGET_EXPORT(kTargetHlsl)
