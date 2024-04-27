// SPDX-License-Identifier: GPL-3.0-only

#include "cfamily-target/target.h"

#include "core/macros.h"
#include "driver/driver.h"

CT_DRIVER_API const target_t kTargetC = {
    .info = {
        .id = "target/cfamily",
        .name = "C",
        .version = {
            .license = "LGPLv3",
            .author = "Elliot Haisley",
            .desc = "C89 output target",
            .version = CT_NEW_VERSION(0, 0, 1)
        }
    },

    .fn_tree = cfamily_tree,
    .fn_ssa = cfamily_ssa
};

CT_TARGET_EXPORT(kTargetC)
