#include "perf.h"

static void do_ref(operand_t op) {
    if (op.kind == ADDRESS) {
        op.block->data = (void*)((size_t)op.block->data + 1);
    }
}

static void do_refs(oplist_t *ops) {
    for (size_t i = 0; i < oplist_len(ops); i++) {
        do_ref(oplist_get(ops, i));
    }
}

static void add_ref(step_t step) {
    switch (step.opcode) {
    case OP_EMPTY: case OP_BLOCK: break;
    case OP_UNARY: case OP_RETURN: do_ref(step.operand); break;
    case OP_BINARY: do_ref(step.lhs); do_ref(step.rhs); break;
    case OP_LOAD: do_ref(step.src); break;
    case OP_STORE: do_ref(step.dst); do_ref(step.src); break;
    case OP_BRANCH: do_ref(step.cond); break;
    case OP_CALL: 
        do_ref(step.func); 
        do_refs(step.args);
        break;
    default: break;
    }
}

static void mark_live_funcs(block_t *block)
{
    for (size_t i = 0; i < block->len; i++) {
        add_ref(block->steps[i]);
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

bool dead_function_elimination(reports_t *reports, module_t *mod) {
    UNUSED(reports);

    bool dirty = false;
    vector_t *funcs = mod->funcs;
    vector_t *vars = mod->vars;
    size_t flen = vector_len(funcs);
    size_t vlen = vector_len(vars);

    logverbose("running DFE pass on %zu blocks in %s", flen + vlen, mod->name);

    for (size_t i = 0; i < flen; i++) {
        block_t *func = vector_get(funcs, i);
        if (func == NULL) { continue; }
        func->data = (void*)(uintptr_t)(can_be_removed(func->attribs));
    }

    for (size_t i = 0; i < flen; i++) {
        block_t *func = vector_get(funcs, i);
        if (func == NULL) { continue; }
        mark_live_funcs(func);
    }

    for (size_t i = 0; i < vlen; i++) {
        block_t *var = vector_get(vars, i);
        if (var == NULL) { continue; }
        mark_live_funcs(var);
    }

    for (size_t i = 0; i < flen; i++) {
        block_t *func = vector_get(funcs, i);
        if (func == NULL) { continue; }
        if (!has_refs(func)) {
            vector_set(funcs, i, NULL);
            dirty = true;
        }
    }

    return dirty;
}

static bool delete_dead_steps(block_t *block) {
    bool dirty = false;
    bool removing = false;
    size_t len = block->len;

    for (size_t i = 0; i < len; i++) {
        step_t *step = block->steps + i;

        if (step->opcode == OP_BLOCK) {
            removing = false;
            continue;
        }

        if (removing && step->opcode != OP_EMPTY) {
            step->opcode = OP_EMPTY;
            dirty = true;
            continue;
        }

        if (step->opcode == OP_JMP || step->opcode == OP_BRANCH || step->opcode == OP_RETURN) {
            removing = true;
        }
    }

    return dirty;
}

bool dead_step_elimination(reports_t *reports, module_t *mod) {
    UNUSED(reports);

    bool dirty = false;
    vector_t *funcs = mod->funcs;
    size_t len = vector_len(funcs);

    logverbose("running DDB pass on %zu blocks in %s", len, mod->name);

    for (size_t i = 0; i < len; i++) {
        block_t *func = vector_get(funcs, i);
        if (func == NULL) { continue; }
        dirty = dirty || delete_dead_steps(func);
    }

    return dirty;
}

static void add_counter(bool *counts, operand_t op) {
    if (op.kind != LABEL) { return; }

    counts[op.label] = true;
}

static void add_counters(bool *counts, step_t step) {
    switch (step.opcode) {
    case OP_JMP: 
        add_counter(counts, step.label); 
        break;
    case OP_BRANCH:
        add_counter(counts, step.label);
        add_counter(counts, step.other);
        break;
    default: break;
    }
}

static bool delete_dead_labels(block_t *block) {
    bool dirty = false;
    size_t len = block->len;
    bool *counts = ctu_malloc(sizeof(bool) * len);
    
    for (size_t i = 0; i < len; i++) {
        counts[i] = false;
    }

    for (size_t i = 0; i < len; i++) {
        step_t step = block->steps[i];
        add_counters(counts, step);
    }

    for (size_t i = 0; i < len; i++) {
        if (block->steps[i].opcode == OP_BLOCK && !counts[i]) {
            block->steps[i].opcode = OP_EMPTY;
            dirty = true;
        }
    }

    ctu_free(counts);

    return dirty;
}

bool dead_label_elimination(reports_t *reports, module_t *mod) {
    UNUSED(reports);

    bool dirty = false;
    size_t len = vector_len(mod->funcs);
    for (size_t i = 0; i < len; i++) {
        block_t *func = vector_get(mod->funcs, i);
        if (func == NULL) { continue; }
        dirty = dirty || delete_dead_labels(func);
    }

    return dirty;
}

void run_passes(reports_t *reports, module_t *ctx) {
    bool needs_more_passes;

    do {
        needs_more_passes = false;
        if (dead_step_elimination(reports, ctx)) {
            needs_more_passes = true;
        }

        if (dead_label_elimination(reports, ctx)) {
            needs_more_passes = true;
        }

        if (dead_function_elimination(reports, ctx)) {
            needs_more_passes = true;
        }
    } while (needs_more_passes);
}
