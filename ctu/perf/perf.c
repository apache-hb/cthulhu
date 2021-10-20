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
static bool can_be_removed(const attrib_t *attribs) {
    return attribs->visibility == ENTRYPOINT 
        || attribs->visibility == PUBLIC;
}

static bool has_refs(const block_t *block) {
    const void *data = block->data;
    uintptr_t count = (uintptr_t)data;
    return count > 0;
}

void dead_function_elimination(reports_t *reports, module_t *mod) {
    UNUSED(reports);

    vector_t *funcs = mod->funcs;
    size_t len = vector_len(funcs);

    logverbose("running DFE pass on %zu blocks in %s", len, mod->name);

    for (size_t i = 0; i < len; i++) {
        block_t *func = vector_get(funcs, i);
        func->data = (void*)(uintptr_t)(can_be_removed(func->attribs));
    }

    for (size_t i = 0; i < len; i++) {
        block_t *func = vector_get(funcs, i);
        mark_live_funcs(func);
    }

    for (size_t i = 0; i < len; i++) {
        block_t *func = vector_get(funcs, i);
        if (!has_refs(func)) {
            vector_set(funcs, i, NULL);
        }
    }
}

static void delete_dead_steps(block_t *block) {
    bool removing = false;
    size_t len = block->len;

    for (size_t i = 0; i < len; i++) {
        step_t *step = block->steps + i;

        if (step->opcode == OP_BLOCK) {
            removing = false;
            continue;
        }

        if (removing) {
            step->opcode = OP_EMPTY;
            continue;
        }

        if (step->opcode == OP_JMP || step->opcode == OP_BRANCH || step->opcode == OP_RETURN) {
            removing = true;
        }
    }
}

void dead_step_elimination(reports_t *reports, module_t *mod) {
    UNUSED(reports);

    vector_t *funcs = mod->funcs;
    size_t len = vector_len(funcs);

    logverbose("running DDB pass on %zu blocks in %s", len, mod->name);

    for (size_t i = 0; i < len; i++) {
        block_t *func = vector_get(funcs, i);
        delete_dead_steps(func);
    }
}

