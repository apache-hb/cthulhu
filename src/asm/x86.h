#pragma once

#include <stddef.h>
#include "ir.h"

typedef enum {
    RAX,
    RBX,
    RCX,
    RDX,

    REG_NONE,
} reg_t;

typedef struct {
    size_t first, /* op where this variable is created */
           last; /* op where this variable is last needed */

    /* associate a register with this range if needed */
    reg_t assoc;
} live_range_t;

typedef struct {
    /* the mangled name of this graph */
    const char *name;
    live_range_t *ranges; /* all ranges for this graph */
    size_t size;
    size_t len;
} live_graph_t;

live_graph_t build_graph(const char *name, unit_t *ir);

void emit_asm(unit_t *ir);
