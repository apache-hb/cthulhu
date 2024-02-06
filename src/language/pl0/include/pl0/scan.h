#pragma once

#include "scan/node.h" // IWYU pragma: export

#define PL0LTYPE where_t

typedef struct logger_t logger_t;

typedef struct pl0_scan_t
{
    logger_t *logger;

    // arena for everything else
    arena_t *arena;

    // arena only for ast nodes
    arena_t *ast_arena;

    // arena for strings
    arena_t *string_arena;
} pl0_scan_t;

pl0_scan_t *pl0_scan_context(scan_t *scan);
