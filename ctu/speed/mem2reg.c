#include "speed.h"

static bool equals_vreg(operand_t op, vreg_t reg) {
    return op.kind == VREG && op.vreg == reg;
}

static size_t count_uses(flow_t *flow, size_t idx, size_t *last) {
    size_t count = 0;
    for (size_t i = idx; i < flow->len; i++) {
        step_t *step = step_at(flow, i);
        if (step->opcode != OP_LOAD && is_vreg_used(step, idx)) {
            count += 1;
            *last = i;
        }
    }

    return count;
}

static void replace_loads(flow_t *flow, size_t start, size_t addr) {
    operand_t vreg = new_vreg(addr);

    step_t *reserve = step_at(flow, start);
    reserve->opcode = OP_VALUE;
    reserve->value = vreg;
    
    for (size_t i = start; i < flow->len; i++) {
        step_t *step = step_at(flow, i);

        if (step->opcode == OP_LOAD && equals_vreg(step->src, start)) {
            step->opcode = OP_VALUE;
            step->value = vreg;
        }
    }
}

bool mem2reg(flow_t *flow) {
    bool dirty = false;
    for (size_t i = 0; i < flow->len; i++) {
        step_t *step = step_at(flow, i);

        if (step->opcode == OP_RESERVE) {
            size_t addr;
            size_t uses = count_uses(flow, i, &addr);

            if (uses == 0) {
                step_at(flow, i)->opcode = OP_EMPTY;
                continue;
            }

            /**
             * duplicate stores to the same variable
             * probably means its mutable.
             * and if its not really thats on you for 
             * writing bad code
             */
            if (uses > 1) {
                continue;
            }

            dirty = true;

            step_t *store = step_at(flow, addr);

            store->opcode = OP_VALUE;
            store->value = store->src;

            replace_loads(flow, i, addr);
        }
    }

    return dirty;
}
