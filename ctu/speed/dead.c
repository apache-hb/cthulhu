#include "speed.h"

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

static void remove_blocks(flow_t *flow, bool *dirty) {
    bool empty = true;
    size_t block = 0;
    
    for (size_t i = 0; i < flow->len; i++) {
        step_t *step = step_at(flow, i);
        if (step->opcode == OP_BLOCK) {
            if (empty) {
                *dirty = true;
                step_at(flow, block)->opcode = OP_EMPTY;
            }
            block = i;
            empty = true;
        } else if (step->opcode != OP_EMPTY) {
            empty = false;
        }
    }
}

bool remove_empty_blocks(module_t *mod) {
    bool dirty = false;

    for (size_t i = 0; i < mod->len; i++) {
        remove_blocks(mod->flows + i, &dirty);
    }

    return dirty;
}