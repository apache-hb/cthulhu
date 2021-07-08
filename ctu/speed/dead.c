#include "speed.h"

#include "ctu/util/report.h"

#include <stdlib.h>

static void remove_ops(flow_t *flow, bool *dirty) {
    /* remove unreachable code after return ops */
    bool dead = false;
    
    for (size_t i = 0; i < flow->len; i++) {
        step_t *step = step_at(flow, i);
        if (step->opcode == OP_BLOCK) {
            dead = false;
        } else if (step->opcode == OP_RETURN || step->opcode == OP_JUMP || step->opcode == OP_BRANCH) {
            dead = true;
        } else if (dead && step->opcode != OP_EMPTY) {
            step->opcode = OP_EMPTY;
            *dirty = true;
        }
    }
}

bool remove_dead_code(module_t *mod) {
    bool dirty = false;

    for (size_t i = 0; i < num_flows(mod); i++) {
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
    } else if (step->opcode == OP_JUMP) {
        track_label(refs, step->block);
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

    for (size_t i = 0; i < num_flows(mod); i++) {
        track_blocks(mod->flows + i, &dirty);
    }

    return dirty;
}

static void update_operand(flow_t *flow, operand_t *op, bool *dirty) {
    if (op->kind == VREG) {
        step_t *step = step_at(flow, op->vreg);
        if (step->opcode == OP_VALUE) {
            *op = step->value;
            *dirty = true;
        }
    }
}

static void propogate_value(flow_t *flow, step_t *step, bool *dirty) {
    switch (step->opcode) {
    case OP_RETURN: case OP_CONVERT: case OP_VALUE: 
    case OP_CALL:
        update_operand(flow, &step->value, dirty); 
        break;
    case OP_UNARY:
        update_operand(flow, &step->expr, dirty); 
        break;
    case OP_BINARY:
        update_operand(flow, &step->lhs, dirty);
        update_operand(flow, &step->rhs, dirty);
        break;
    case OP_BRANCH:
        update_operand(flow, &step->cond, dirty); 
        break;
    case OP_STORE:
        update_operand(flow, &step->src, dirty);
        break;

    default:
        break;
    }
}

static void propogate_flow(flow_t *flow, bool *dirty) {
    for (size_t i = 0; i < flow->len; i++) {
        step_t *step = step_at(flow, i);
        propogate_value(flow, step, dirty);
    }
}

bool propogate_consts(module_t *mod) {
    bool dirty = false;

    for (size_t i = 0; i < num_flows(mod); i++) {
        propogate_flow(mod->flows + i, &dirty);
    }

    return dirty;
}

static bool is_value_used(flow_t *flow, size_t vreg) {
    for (size_t i = vreg; i < flow->len; i++) {
        if (is_vreg_used(step_at(flow, i), vreg)) {
            return true;
        }
    }
    return false;
}

static void flow_remove_unused(flow_t *flow, bool *dirty) {
    for (size_t i = 0; i < flow->len; i++) {
        step_t *step = step_at(flow, i);
        if (step->opcode != OP_VALUE)
            continue;

        if (!is_value_used(flow, i)) {
            step->opcode = OP_EMPTY;
            *dirty = true;
        }
    }
}

bool remove_unused_code(module_t *mod) {
    bool dirty = false;

    for (size_t i = 0; i < num_flows(mod); i++) {
        flow_remove_unused(mod->flows + i, &dirty);
    }

    return dirty;
}

static bool is_block_empty(flow_t *flow, size_t idx, size_t *end) {
    
    for (size_t i = idx + 1; i < flow->len; i++) {
        step_t *step = step_at(flow, i);
        
        if (step->opcode == OP_BLOCK) {
            *end = i;
            return true;
        }
        
        if (step->opcode != OP_EMPTY)
            return false;
    }

    *end = flow->len;
    return true;
} 

static bool is_block(operand_t op, size_t label) {
    return op.kind == BLOCK && op.label == label;
}

static void replace_branch_leafs(flow_t *flow, size_t old, size_t replace) {
    for (size_t i = 0; i < flow->len; i++) {
        step_t *step = step_at(flow, i);

        if (step->opcode == OP_BRANCH) {
            if (is_block(step->block, old)) {
                step->block = new_block(replace);
            }

            if (is_block(step->other, old)) {
                step->other = new_block(replace);
            }
        }
    }
}

static void remove_empty(flow_t *flow, bool *dirty) {
    size_t replace;

    for (size_t i = 0; i < flow->len; i++) {
        step_t *step = step_at(flow, i);
        if (step->opcode != OP_BLOCK)
            continue;

        /**
         * if this block is empty then replace
         * all references to this block with the
         * end of the block
         */
        if (is_block_empty(flow, i, &replace)) {
            step->opcode = OP_EMPTY;
            *dirty = true;
            replace_branch_leafs(flow, i, replace);
        }
    }
}

bool remove_empty_blocks(module_t *mod) {
    bool dirty = false;

    for (size_t i = 0; i < num_flows(mod); i++) {
        remove_empty(mod->flows + i, &dirty);
    }

    return dirty;
}

static bool blocks_equal(operand_t lhs, operand_t rhs) {
    return lhs.kind == BLOCK 
        && rhs.kind == BLOCK
        && lhs.label == rhs.label;
}

static bool is_const_true(operand_t op) {
    return op.kind == IMM
        && op.imm.kind == IMM_BOOL
        && op.imm.imm_bool;
}

static void remove_unneeded_branches(flow_t *flow, bool *dirty) {
    for (size_t i = 0; i < flow->len; i++) {
        step_t *step = step_at(flow, i);

        if (step->opcode != OP_BRANCH)
            continue;

        if (blocks_equal(step->block, step->other)) {
            *step = new_jump(step->block);
            *dirty = true;
        } else if (is_const_true(step->cond)) {
            *step = new_jump(step->block);
            *dirty = true;
        }
    }
}

bool remove_branches(module_t *mod) {
    bool dirty = false;

    for (size_t i = 0; i < num_flows(mod); i++) {
        remove_unneeded_branches(mod->flows + i, &dirty);
    }

    return dirty;   
}

static bool jump_skips_steps(flow_t *flow, size_t idx) {
    step_t *jump = step_at(flow, idx);
    if (jump->block.kind != BLOCK)
        return true;

    size_t to = jump->block.label;

    for (size_t i = idx + 1; i < to; i++) {
        step_t *step = step_at(flow, i);

        if (step->opcode != OP_EMPTY)
            return true;
    }

    return false;
}

static void remove_unused_jumps(flow_t *flow, bool *dirty) {
    for (size_t i = 0; i < flow->len; i++) {
        step_t *step = step_at(flow, i);

        if (step->opcode != OP_JUMP)
            continue;

        if (!jump_skips_steps(flow, i)) {
            step->opcode = OP_EMPTY;
            *dirty = true;
        }
    }
}

bool remove_jumps(module_t *mod) {
    bool dirty = false;

    for (size_t i = 0; i < num_flows(mod); i++) {
        remove_unused_jumps(mod->flows + i, &dirty);
    }

    return dirty; 
}

static bool is_pure_op(step_t *step) {
    return step->opcode == OP_BINARY 
        || step->opcode == OP_UNARY
        || step->opcode == OP_CONVERT
        || step->opcode == OP_VALUE
        || step->opcode == OP_LOAD;
}

static void remove_code(flow_t *flow, bool *dirty) {
    for (size_t i = 0; i < flow->len; i++) {
        step_t *step = step_at(flow, i);
        if (!is_pure_op(step))
            continue;

        if (step->opcode == OP_EMPTY)
            continue;

        if (!is_value_used(flow, i)) {
            step->opcode = OP_EMPTY;
            *dirty = true;
        }
    }
}

bool remove_pure_code(module_t *mod) {
    bool dirty = false;

    for (size_t i = 0; i < num_flows(mod); i++) {
        remove_code(mod->flows + i, &dirty);
    }

    return dirty; 
}