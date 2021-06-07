#include "regalloc.h"

#include <stdbool.h>

struct packet_t { 
    bool used;
    size_t idx;
};

static void operand_uses(operand_t op, struct packet_t *packet) {
    packet->used = packet->used || (op.type == REG ? op.reg == packet->idx : false);
}

/* check if `op` references `self` */
static bool opcode_uses(opcode_t *op, size_t self) {
    struct packet_t packet = { false, self };
    APPLY_GENERIC(op, operand_uses, &packet);
    return packet.used;
}

alloc_t get_range(unit_t *unit, size_t idx) {
    /* track the total range of this opcode */
    size_t last = idx;

    for (size_t i = idx; i < unit->len; i++) {
        opcode_t *op = ir_opcode_at(unit, i);
        if (opcode_uses(op, idx)) {
            last = i;
        }
    }

    alloc_t alloc = { ALNULL, idx, last, { } };

    return alloc;
}

static bool reg_is_live(alloc_t range, size_t idx) {
    return range.first <= idx && idx <= range.last;
}

static void clean_regs(regalloc_t *alloc, size_t idx) {
    for (size_t i = 0; i < alloc->regs; i++) {
        /* check if this register is currently in use */
        size_t *reg = alloc->used + i;

        if (*reg != UNUSED_REG && reg_is_live(alloc->data[*reg], idx)) {
            /* if its not then release the register */
            *reg = UNUSED_REG;
        }
    }
}

static void patch_alloc_reg(alloc_t *alloc, size_t reg) {
    alloc->type = ALREG;
    alloc->reg = reg;
}

static void patch_alloc_spill(alloc_t *alloc, size_t addr) {
    alloc->type = ALSPILL;
    alloc->addr = addr;
}

static void assign_alloc(regalloc_t *alloc, size_t idx) {
    clean_regs(alloc, idx);

    alloc_t *it = alloc->data + idx;

    for (size_t i = 0; i < alloc->regs; i++) {
        size_t *reg = alloc->used + i;
        if (*reg == UNUSED_REG) {
            *reg = idx;
            patch_alloc_reg(it, i);
            return;
        }
    }

    /* TODO: reuse spill when possible */
    patch_alloc_spill(it, alloc->stack++);
}

static regalloc_t new_regalloc(size_t len, size_t regs) {
    regalloc_t alloc = {
        malloc(sizeof(size_t) * regs), regs, 
        0, malloc(sizeof(alloc_t) * len)
    };

    for (size_t i = 0; i < regs; i++)
        alloc.used[i] = UNUSED_REG;

    return alloc;
}

regalloc_t regalloc_assign(unit_t *unit, size_t regs) {
    regalloc_t alloc = new_regalloc(unit->len, regs);

    for (size_t i = 0; i < unit->len; i++) {
        alloc.data[i] = get_range(unit, i);
    }

    for (size_t i = 0; i < unit->len; i++) {
        assign_alloc(&alloc, i);
    }

    return alloc;
}
