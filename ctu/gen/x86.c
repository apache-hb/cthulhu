#include "x86.h"

#include "ctu/ir/ir.h"
#include "ctu/util/report.h"

#include <stdlib.h>
#include <stdio.h>

typedef enum {
    RAX, RBX, RCX, RDX,
    RSI, RDI, RBP, RIP,
    RSP, 

    R8, R9, R10, R11,
    R12, R13, R14, R15,

    USABLE,

    UNUSED
} reg_t;

/**
 * an allocation
 */
typedef struct {
    enum { REG, SPILL } kind;
    union {
        reg_t reg;
        uint64_t spill;
    };
} alloc_t;

static alloc_t alloc_reg(reg_t reg) {
    alloc_t alloc = { REG, { reg } };
    return alloc;
}

static const char *alloc_tostr(alloc_t alloc) {
    if (alloc.kind == REG) {
        switch (alloc.reg) {
        case RAX: return "rax";
        case RBX: return "rbx";
        case RCX: return "rcx";
        case RDX: return "rdx";
        case UNUSED: return "unused";
        default: return "ERR";
        }
    } else {
        return "spill";
    }
}

/**
 * a range where a register is live
 */
typedef struct {
    size_t start;
    size_t end;

    /**
     * the ssa register this range is associated with
     */
    size_t vreg;

    /**
     * does this variable *need* to be a memory operand
     * this is true for anything that has a reference taken
     * to it for now
     */
    bool mem:1;

    /** 
     * the actual allocation 
     */
    alloc_t alloc;
} range_t;

#define REG_MAX USABLE

typedef struct {
    flow_t *flow;

    range_t *ranges;
    size_t len;

    /** 
     * current used registers 
     * 
     * either SIZE_MAX when not in use
     * or an index into ranges
     */
    size_t used[REG_MAX];

    vreg_t *spill;
    size_t total;
} x86_regalloc_t;

static x86_regalloc_t new_regalloc(flow_t *flow) {
    x86_regalloc_t alloc;

    alloc.flow = flow;

    alloc.ranges = malloc(sizeof(range_t) * flow->len);
    alloc.len = flow->len;

    alloc.spill = malloc(sizeof(vreg_t) * 4);
    alloc.total = 0;

    for (size_t i = 0; i < REG_MAX; i++) {
        alloc.used[i] = SIZE_MAX;
    }

    return alloc;
}


static void assign_range(x86_regalloc_t *regs, size_t idx) {
    size_t start = idx;
    size_t end = start;
    bool mem = false;
    
    step_t *first = step_at(regs->flow, idx);

    if (first != OP_EMPTY)  {
        for (size_t i = idx; i < regs->flow->len; i++) {
            if (is_vreg_used(step_at(regs->flow, i), idx)) {
                end = i;
            }
        }
        mem = first->opcode == OP_RESERVE;
    }

    range_t range = { start, end, idx, mem, alloc_reg(USABLE) };
    regs->ranges[idx] = range;
}

static bool reg_is_used(x86_regalloc_t *alloc, reg_t reg) {
    return alloc->used[reg] != SIZE_MAX;
}

static range_t range_of_reg(x86_regalloc_t *alloc, reg_t reg) {
    ASSERT(reg_is_used(alloc, reg))("cannot query range of unused register");

    size_t range = alloc->used[reg];
    range_t out = alloc->ranges[range];
    return out;
}

static void clean_reg(x86_regalloc_t *alloc, reg_t reg, size_t end) {
    if (reg_is_used(alloc, reg) && range_of_reg(alloc, reg).end <= end) {
        alloc->used[reg] = SIZE_MAX;
    }
}

/**
 * boot out any registers that are no longer needed
 * at end
 */
static void clean_all_regs(x86_regalloc_t *alloc, size_t end) {
    for (size_t reg = 0; reg < REG_MAX; reg++) {
        clean_reg(alloc, reg, end);
    }
}

static reg_t get_free_reg(x86_regalloc_t *alloc) {
    for (size_t reg = 0; reg < REG_MAX; reg++) {
        if (!reg_is_used(alloc, reg)) {
            return reg;
        }
    }

    assert("failed to find free register");
    return 0;
}

static bool range_needs_reg(range_t range) {
    return range.start < range.end;
}

static void assign_reg(x86_regalloc_t *alloc, size_t idx) {
    clean_all_regs(alloc, idx);
    
    reg_t reg = UNUSED;

    if (range_needs_reg(alloc->ranges[idx])) {
        reg = get_free_reg(alloc);
        alloc->used[reg] = idx;
    }

    alloc->ranges[idx].alloc = alloc_reg(reg);
}

static x86_regalloc_t assign_ranges(flow_t *flow) {
    x86_regalloc_t regalloc = new_regalloc(flow);

    for (size_t i = 0; i < flow->len; i++) {
        assign_range(&regalloc, i);
    }

    for (size_t i = 0; i < flow->len; i++) {
        assign_reg(&regalloc, i);
    }

    printf("%s\n", flow->name);
    for (size_t i = 0; i < regalloc.len; i++) {
        range_t range = regalloc.ranges[i];
        printf("- %zu: %zu -> %zu = %s\n", i, range.start, range.end, alloc_tostr(range.alloc));
    }

    return regalloc;
}

blob_t *gen_x64(module_t *mod) {
    blob_t *blob = new_blob();

    for (size_t i = 0; i < num_flows(mod); i++) {
        assign_ranges(mod->flows + i);
    }
    return blob;
}
