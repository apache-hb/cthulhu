// SPDX-License-Identifier: GPL-3.0-only

#include "llvm-target/target.h"

#include "driver/driver.h"

#define NEW_EVENT(id, ...) const diagnostic_t kEvent_##id = __VA_ARGS__;
#include "llvm-target/events.inc"

static const diagnostic_t *const kDiagnosticTable[] = {
#define NEW_EVENT(id, ...) &kEvent_##id,
#include "llvm-target/events.inc"
};

CT_DRIVER_API const target_t kTargetLLVM = {
    .info = {
        .id = "target/llvm",
        .name = "C",
        .version = {
            .license = "LGPLv3",
            .author = "Elliot Haisley",
            .desc = "LLVM output target",
            .version = CT_NEW_VERSION(0, 0, 1)
        },

        .diagnostics = {
            .diagnostics = kDiagnosticTable,
            .count = sizeof(kDiagnosticTable) / sizeof(const diagnostic_t *)
        }
    },

    .fn_create = llvm_create,
    .fn_destroy = llvm_destroy,

    .fn_ssa = llvm_ssa
};

CT_TARGET_EXPORT(kTargetLLVM)
