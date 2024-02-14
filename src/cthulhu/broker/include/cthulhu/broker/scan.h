#pragma once

typedef struct logger_t logger_t;
typedef struct arena_t arena_t;

typedef struct lang_scan_t
{
    logger_t *logger;

    arena_t *arena;
    arena_t *string_arena;
    arena_t *ast_arena;
} lang_scan_t;
