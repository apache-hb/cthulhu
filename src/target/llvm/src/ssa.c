// SPDX-License-Identifier: LGPL-3.0-or-later

#include "llvm-target/target.h"

#include "cthulhu/ssa/ssa.h"

#include "core/macros.h"
#include "std/vector.h"

#include "llvm-c/Core.h"
#include "llvm-c/Target.h"

static LLVMContextRef gContext = NULL;

void llvm_create(target_runtime_t *runtime)
{
    CT_UNUSED(runtime);

    LLVMInitializeAllTargets();

    gContext = LLVMContextCreate();
}

void llvm_destroy(target_runtime_t *runtime)
{
    // empty
    CT_UNUSED(runtime);

    LLVMContextDispose(gContext);

    LLVMShutdown();
}

typedef struct llvm_target_t {
    void *stub;

    vector_t *path;
} llvm_target_t;

static void llvm_build_module(llvm_target_t *self, const ssa_module_t *mod)
{

}

void llvm_ssa(target_runtime_t *runtime, const ssa_result_t *ssa, target_emit_t *emit)
{
    vector_t *mods = ssa->modules;

    CT_UNUSED(runtime);
    CT_UNUSED(mods);
    CT_UNUSED(emit);

    llvm_target_t info = {
        .stub = NULL
    };

    size_t len = vector_len(mods);
    for (size_t i = 0; i < len; i++)
    {

    }
}
