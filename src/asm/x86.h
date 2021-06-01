#pragma once

#include <stddef.h>
#include "ir.h"

typedef enum {
    RAX, RBX, RCX, RDX
} x86_reg_t;

typedef struct {
    size_t first, /* op where this variable is created */
           last; /* op where this variable is last needed */
} live_range_t;

typedef struct {
    live_range_t *ranges; /* all ranges for this graph */
    size_t size;
    size_t len;
} live_graph_t;

live_graph_t build_graph(unit_t *ir);
