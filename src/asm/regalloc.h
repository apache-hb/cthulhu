#pragma once

#include "ir.h"
#include <stddef.h>
#include <limits.h>

typedef struct {
    enum { ALREG, ALSPILL, ALNULL = INT_MAX } type;

    size_t first, last;

    union {
        int reg;
        size_t addr;
    };
} alloc_t;

#define UNUSED_REG SIZE_MAX

typedef struct {
    /* either points to the opcode thats currently using this register, or SIZE_MAX */
    size_t *slots;
    /* the total number of register slots */
    size_t regs;

    /* stack slots */
    size_t *stack;
    /* total spill size */
    size_t spill;
    /* max spill slots */
    size_t used;

    /* all allocations */
    alloc_t *data;
} regalloc_t;

regalloc_t regalloc_assign(unit_t *unit, size_t regs);
