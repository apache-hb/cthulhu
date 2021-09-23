#include "llvm.h"

#include <llvm-c/Core.h>
#include <llvm-c/TargetMachine.h>

typedef struct {
    module_t *mod;
    reports_t *reports;

    LLVMModuleRef llvm_module;
} llvm_context_t;

static llvm_context_t *init_llvm_context(module_t *mod, reports_t *reports, const char *path) {
    llvm_context_t *ctx = NEW(llvm_context_t);

    ctx->mod = mod;
    ctx->reports = reports;

    ctx->llvm_module = LLVMModuleCreateWithName(path);

    return ctx;
}

bool llvm_build(reports_t *reports, module_t *mod, const char *path) {
    llvm_context_t *ctx = init_llvm_context(mod, reports, path);

    return ctx != NULL;
}
