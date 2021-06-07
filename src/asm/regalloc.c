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

static bool alloc_is_live(alloc_t range, size_t idx) {
    return range.first <= idx && idx < range.last;
}

static bool assign_slot(size_t *spill, size_t idx) {
    if (*spill == UNUSED_REG) {
        *spill = idx;
        return true;
    }
    return false;
}

static void patch_alloc_reg(alloc_t *alloc, size_t reg) {
    alloc->type = ALREG;
    alloc->reg = reg;
}

static void patch_alloc_spill(alloc_t *alloc, size_t addr) {
    alloc->type = ALSPILL;
    alloc->addr = addr;
}

static void add_spill_slot(regalloc_t *alloc, alloc_t *it) {
    if (alloc->used > alloc->spill) {
        alloc->spill += 4;
        alloc->stack = realloc(alloc->stack, sizeof(size_t) * alloc->spill);
    }

    size_t slot = alloc->used++;
    alloc->stack[slot] = slot;
    patch_alloc_spill(it, slot);
}

static void clean_slots(
    regalloc_t *alloc, size_t len,
    size_t* slots, size_t idx) {
    
    /* check if this slot is currently in used */
    for (size_t i = 0; i < len; i++) {
        size_t *slot = slots + i;
        /* if its not then release the slot */
        if (!alloc_is_live(alloc->data[*slot], idx)) {
            *slot = UNUSED_REG;
        }
    }
}

static void clean_regs(regalloc_t *alloc, size_t idx) {
    clean_slots(alloc, alloc->regs, alloc->slots, idx);
}

static void clean_spill(regalloc_t *alloc, size_t idx) {
    clean_slots(alloc, alloc->used, alloc->stack, idx);
}

static size_t find_free_slot(size_t len, size_t *slots, size_t idx) {
    for (size_t i = 0; i < len; i++) {
        if (assign_slot(slots + i, idx)) {
            return i;
        }
    }

    return SIZE_MAX;
}

static void assign_alloc(regalloc_t *alloc, size_t idx) {
    alloc_t *it = alloc->data + idx;
    /* if the range is empty then this instruction doesnt need a register */
    if (it->first == it->last)
        return;

    clean_regs(alloc, idx);

    size_t reg = find_free_slot(alloc->regs, alloc->slots, idx);
    if (reg != SIZE_MAX) {
        patch_alloc_reg(it, reg);
        return;
    }

    clean_spill(alloc, idx);

    size_t spill = find_free_slot(alloc->used, alloc->stack, idx);
    if (spill != SIZE_MAX) {
        patch_alloc_spill(it, spill);
        return;
    }

    add_spill_slot(alloc, it);
}

#define INIT_SPILL 4

static regalloc_t new_regalloc(size_t len, size_t regs) {
    regalloc_t alloc = {
        /* register slots */
        malloc(sizeof(size_t) * regs), regs, 
        
        /* stack slots */
        malloc(sizeof(size_t) * INIT_SPILL), INIT_SPILL, 0,

        /* allocations */
        malloc(sizeof(alloc_t) * len)
    };

    for (size_t i = 0; i < regs; i++)
        alloc.slots[i] = UNUSED_REG;

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
