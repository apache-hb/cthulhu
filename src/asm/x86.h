#pragma once

#include <stddef.h>
#include <stdio.h>
#include "ir.h"

typedef enum {
    RAX,
    RBX,
    RCX,
    RDX,

    REG_NONE,

    /* not used in general regalloc yet */
    RDI,
    RSI,
    R8,
    R9
} reg_t;

/* an allocated register */
typedef struct {
    enum { IN_REG, SPILL } type;

    union {
        /* the register this is stored in */
        reg_t reg;
        /* if not stored in a register than an offset into the stack */
        size_t offset;
    };
} alloc_t;

typedef struct {
    size_t first, /* op where this variable is created */
           last; /* op where this variable is last needed */

    /* associate a register with this range if needed */
    alloc_t assoc;
} live_range_t;

typedef struct {
    live_range_t *ranges; /* all ranges for this graph */
    size_t size;
    size_t len;
} live_graph_t;

live_graph_t build_graph(unit_t *ir);

void emit_asm(unit_t *ir, FILE *output);
