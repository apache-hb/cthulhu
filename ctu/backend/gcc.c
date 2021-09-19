#include "gcc.h"

#include <libgccgit.h>

typedef struct {
    reports_t *reports;
    module_t *module;
    gcc_jit_context *context;
} context_t;

context_t *context_init(reports_t *reports, module_t *module) {
    gcc_jit_context *gcc = gcc_jit_context_acquire();

    if (gcc == NULL) {
        assert2(reports, "failed to create gccjit context");
        return NULL;
    }

    context_t context = NEW(context_t); 
    
    context->reports = reports;
    context->module = module;
    context->context = gcc;

    return context;
}

bool gccjit_build(reports_t *reports, module_t *mod, const char *path) {
    context_t *context = context_init(reports, mod);
    if (context == NULL) {
        return false;
    }

    

    return true;
}
