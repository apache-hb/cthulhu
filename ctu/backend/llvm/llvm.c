#include "llvm.h"

#include <llvm-c/Core.h>
#include <llvm-c/TargetMachine.h>

typedef struct {
    module_t *mod;
    reports_t *reports;

    LLVMModuleRef llvm_module;
} llvm_context_t;

static llvm_context_t *init_llvm_context(module_t *mod, reports_t *reports, path_t *path) {
    llvm_context_t *ctx = ctu_malloc(sizeof(llvm_context_t));

    ctx->mod = mod;
    ctx->reports = reports;

    ctx->llvm_module = LLVMModuleCreateWithName(path_realpath(path));

    return ctx;
}

bool llvm_build(reports_t *reports, module_t *mod, const char *path) {
    llvm_context_t *ctx = init_llvm_context(mod, reports, path);

    return ctx != NULL;
}
