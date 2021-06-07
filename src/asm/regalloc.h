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
    size_t *used;
    size_t regs;

    size_t stack;

    alloc_t *data;
} regalloc_t;

regalloc_t regalloc_assign(unit_t *unit, size_t regs);
