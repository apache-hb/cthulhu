#pragma once

#include "ir.h"
#include <stddef.h>
#include <limits.h>

#define UNUSED_REG SIZE_MAX
#define NO_PHI SIZE_MAX

typedef size_t slot_t;

typedef struct {
    enum { ALREG, ALSPILL, ALNULL = INT_MAX } type;

    /* the first and last time this allocation is used */
    size_t first, last;

    /* where this allocation is merged if it is merged */
    size_t phi;

    /* either the register index or the spill index */
    size_t reg;
} alloc_t;

typedef struct {
    /* either points to the opcode thats currently using this register, or SIZE_MAX */
    slot_t *slots;
    /* the total number of register slots */
    size_t regs;

    /* stack slots */
    slot_t *stack;
    /* total spill size */
    size_t spill;
    /* max spill slots */
    size_t used;

    /* all allocations */
    alloc_t *data;
    size_t total;
} regalloc_t;

regalloc_t regalloc_assign(unit_t *unit, size_t regs);
