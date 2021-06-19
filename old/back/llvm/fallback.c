#include "llvm.h"
#include "debug.h"

#include "cthulhu/util/report.h"

bool llvm_enabled(void) {
    return false;
}

void llvm_debug(llvm_context *ctx) {
    (void)ctx;
    reportf("llvm is disabled");
}

llvm_context *llvm_compile(unit_t *unit) {
    (void)unit;
    reportf("llvm is disabled");
    return NULL;
}

void llvm_output(llvm_context *ctx, FILE *file) {
    (void)ctx;
    (void)file;
    reportf("llvm is disabled");
}
