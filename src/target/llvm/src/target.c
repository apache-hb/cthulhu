// SPDX-License-Identifier: GPL-3.0-only

#include "llvm-target/target.h"

#include "driver/driver.h"

CT_DRIVER_API const target_t kTargetLLVM = {
    .info = {
        .id = "target/llvm",
        .name = "C",
        .version = {
            .license = "LGPLv3",
            .author = "Elliot Haisley",
            .desc = "LLVM output target",
            .version = CT_NEW_VERSION(0, 0, 1)
        }
    },

    .fn_create = llvm_create,
    .fn_destroy = llvm_destroy,

    .fn_ssa = llvm_ssa
};

CT_TARGET_EXPORT(kTargetLLVM)
