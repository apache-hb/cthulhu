#include "speed.h"

#include <stdlib.h>

static void remove_ops(flow_t *flow, bool *dirty) {
    /* remove unreachable code after return ops */
    bool dead = false;
    
    for (size_t i = 0; i < flow->len; i++) {
        step_t *step = step_at(flow, i);
        if (step->opcode == OP_BLOCK) {
            dead = false;
            continue;
        }

        if (dead) {
            step->opcode = OP_EMPTY;
            *dirty = true;
        }

        if (step->opcode == OP_RETURN) {
            dead = true;
        }
    }
}

bool remove_dead_code(module_t *mod) {
    bool dirty = false;

    for (size_t i = 0; i < mod->len; i++) {
        remove_ops(mod->flows + i, &dirty);
    }

    return dirty;
}

static void track_label(bool *refs, operand_t op) {
    if (op.kind != BLOCK)
        return;

    refs[op.label] = true;
}

static void track_branch(bool *refs, size_t idx, step_t *step) {
    if (step->opcode == OP_BRANCH) { 
        track_label(refs, step->block);
        track_label(refs, step->other);
        refs[idx] = true;
    } else {
        refs[idx] |= step->opcode != OP_BLOCK;
    }
}

static void track_blocks(flow_t *flow, bool *dirty) {
    bool *refs = malloc(sizeof(bool) * flow->len);
    for (size_t i = 0; i < flow->len; i++)
        refs[i] = false;

    for (size_t i = 0; i < flow->len; i++) {
        track_branch(refs, i, step_at(flow, i));
    }

    for (size_t i = 0; i < flow->len; i++) {
        if (!refs[i]) {
            step_at(flow, i)->opcode = OP_EMPTY;
            *dirty = true;
        }
    }

    free(refs);
}

bool remove_unused_blocks(module_t *mod) {
    bool dirty = false;

    for (size_t i = 0; i < mod->len; i++) {
        track_blocks(mod->flows + i, &dirty);
    }

    return dirty;
}
