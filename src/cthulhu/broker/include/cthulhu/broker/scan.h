#pragma once

#include <ctu_broker_api.h>

typedef struct logger_t logger_t;
typedef struct arena_t arena_t;
typedef struct scan_t scan_t;

typedef struct scan_context_t
{
    logger_t *logger;

    arena_t *arena;
    arena_t *string_arena;
    arena_t *ast_arena;

    char user[];
} scan_context_t;

CT_BEGIN_API

CT_BROKER_API logger_t *get_logger(const scan_t *scan);
CT_BROKER_API arena_t *get_arena(const scan_t *scan);
CT_BROKER_API arena_t *get_string_arena(const scan_t *scan);
CT_BROKER_API arena_t *get_ast_arena(const scan_t *scan);

CT_BROKER_API void *get_user(const scan_t *scan);

CT_END_API
