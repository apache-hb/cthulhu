#include "perf.h"

static void add_ref(operand_t op) {
    if (op.kind == ADDRESS) {
        op.block->data = (void*)((size_t)op.block->data) + 1; 
    }
}

static void mark_live_funcs(block_t *block)
{
    for (size_t i = 0; i < block->len; i++) {
        step_t step = block->steps[i];
        if (step.opcode == OP_CALL) {
            add_ref(step.func);
        }
    }
}

void dead_function_elimination(reports_t *reports, 
                               module_t *mod)
{
    vector_t *funcs = mod->funcs;
    size_t len = vector_len(funcs);

    report2(reports, NOTE, NULL, "running DFE pass on %zu blocks", len);

    for (size_t i = 0; i < len; i++) {
        block_t *func = vector_get(funcs, i);
        func->data = (void*)(uintptr_t)(func->exported ? 1 : 0);
    }

    for (size_t i = 0; i < len; i++) {
        block_t *func = vector_get(funcs, i);
        mark_live_funcs(func);
    }

    for (size_t i = 0; i < len; i++) {
        block_t *func = vector_get(funcs, i);
        if ((size_t)func->data == 0) {
            block_free(func);
            vector_set(funcs, i, NULL);
        }
    }
}
