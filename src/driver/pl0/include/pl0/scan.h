#pragma once

#include "scan/node.h" // IWYU pragma: export

#define PL0LTYPE where_t

typedef struct logger_t logger_t;

typedef struct pl0_scan_t
{
    arena_t *arena;
    logger_t *reports;
} pl0_scan_t;

pl0_scan_t *pl0_scan_context(scan_t *scan);
