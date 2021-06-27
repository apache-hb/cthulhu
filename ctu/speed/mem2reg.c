#include "speed.h"

static bool equals_vreg(operand_t op, vreg_t reg) {
    return op.kind == VREG && op.vreg == reg;
}

static bool stores_to(step_t *step, size_t idx) {
    if (step->opcode != OP_STORE) {
        return false;
    }

    return equals_vreg(step->dst, idx);
}

static size_t count_stores(flow_t *flow, size_t idx, size_t *last) {
    size_t count = 0;
    
    for (size_t i = idx; i < flow->len; i++) {
        if (stores_to(step_at(flow, i), idx)) {
            count += 1;
            *last = i;
        }
    }

    return count;
}

static void replace_loads(flow_t *flow, size_t start, size_t addr) {
    for (size_t i = start; i < flow->len; i++) {
        step_t *step = step_at(flow, i);

        if (step->opcode == OP_LOAD) {
            step->opcode = OP_VALUE;
            step->value = new_vreg(addr);
        }
    }
}

static void mem2reg_flow(flow_t *flow, bool *dirty) {
    for (size_t i = 0; i < flow->len; i++) {
        step_t *step = step_at(flow, i);

        if (step->opcode == OP_RESERVE) {
            size_t addr;
            size_t stores = count_stores(flow, i, &addr);

            /**
             * duplicate stores to the same variable
             * probably means its mutable.
             * and if its not really thats on you for 
             * writing bad code
             */
            if (stores > 1) {
                continue;
            }

            *dirty = true;

            step_t *store = step_at(flow, addr);

            store->opcode = OP_VALUE;
            store->value = store->src;

            replace_loads(flow, i, addr);

            step->opcode = OP_EMPTY;
        }
    }
}

bool mem2reg(module_t *mod) {
    bool dirty = false;

    for (size_t i = 0; i < num_flows(mod); i++) {
        mem2reg_flow(mod->flows + i, &dirty);
    }

    return dirty;
}
