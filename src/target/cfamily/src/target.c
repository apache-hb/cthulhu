// SPDX-License-Identifier: GPL-3.0-only

#include "cfamily/target.h"

#include "core/macros.h"
#include "driver/driver.h"

static void cfamily_create(target_runtime_t *runtime)
{
    // empty
    CT_UNUSED(runtime);
}

static void cfamily_destroy(target_runtime_t *runtime)
{
    // empty
    CT_UNUSED(runtime);
}

static void cfamily_tree(target_runtime_t *runtime, const tree_t *tree, target_emit_t *emit)
{
    // empty
    CT_UNUSED(runtime);
    CT_UNUSED(tree);
    CT_UNUSED(emit);
}

static void cfamily_ssa(target_runtime_t *runtime, const ssa_result_t *ssa, target_emit_t *emit)
{
    // empty
    CT_UNUSED(runtime);
    CT_UNUSED(ssa);
    CT_UNUSED(emit);
}

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

    .fn_create = cfamily_create,
    .fn_destroy = cfamily_destroy,

    .fn_tree = cfamily_tree,
    .fn_ssa = cfamily_ssa
};

CT_TARGET_EXPORT(kTargetC)
