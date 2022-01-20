#pragma once

#include "scan.h"
#include "cthulhu/util/macros.h"

/* a position in a source file */
typedef struct {
    scan_t *scan;
    where_t where;
} node_t;

node_t *node_new(scan_t *scan, where_t where) NONULL;
node_t *node_builtin(scan_t *scan);
node_t *node_last_line(const node_t *node) NONULL;
node_t *node_merge(const node_t *lhs, const node_t *rhs) NONULL;
