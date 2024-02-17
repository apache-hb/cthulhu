#pragma once

#include "scan/node.h" // IWYU pragma: export

#define METALTYPE where_t

typedef struct logger_t logger_t;

typedef struct meta_scan_t
{
    logger_t *logger;
    arena_t *arena;
} meta_scan_t;

meta_scan_t *meta_scan_context(scan_t *scan);
