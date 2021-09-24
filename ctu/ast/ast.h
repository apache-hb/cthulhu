#pragma once

#include "scan.h"

/* a position in a source file */
typedef struct {
    scan_t *scan;
    where_t where;
} node_t;

node_t *node_new(scan_t *scan, where_t where);
void node_delete(node_t *node);
