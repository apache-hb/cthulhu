#include "gcc.h"

#include <libgccjit.h>

typedef struct {
    reports_t *reports;
    module_t *module;
    gcc_jit_context *context;
} context_t;

typedef struct {
    block_t *block;

    void **steps;
} gcc_block_t;

static gcc_block_t *block_init(block_t *block) {
    gcc_block_t *gcc_block = NEW(gcc_block_t);
    gcc_block->block = block;
    gcc_block->steps = NEW_ARRAY(void*, block->len);

    block->data = gcc_block;

    return gcc_block;
}

static context_t *context_init(reports_t *reports, module_t *module) {
    gcc_jit_context *gcc = gcc_jit_context_acquire();

    if (gcc == NULL) {
        assert2(reports, "failed to create gccjit context");
        return NULL;
    }

    context_t *context = NEW(context_t); 
    
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

    vector_t *funcs = mod->funcs;
    for (size_t i = 0; i < vector_len(funcs); i++) {
        block_init(vector_get(funcs, i));
    }

    gcc_jit_context_compile_to_file(
        context->context,
        GCC_JIT_OUTPUT_KIND_EXECUTABLE,
        path   
    );

    return true;
}
