#pragma once

#include "scan.h"
#include "ctu/util/macros.h"

/* a position in a source file */
typedef struct {
    WEAK scan_t *scan;
    where_t where;
} node_t;

node_t *node_new(WEAK scan_t *scan, where_t where) NONULL;
