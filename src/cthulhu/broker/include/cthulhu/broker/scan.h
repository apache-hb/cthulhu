#pragma once

#include <ctu_broker_api.h>

typedef struct logger_t logger_t;
typedef struct arena_t arena_t;
typedef struct scan_t scan_t;
typedef struct where_t where_t;

typedef struct scan_context_t
{
    logger_t *logger;

    arena_t *arena;
    arena_t *string_arena;
    arena_t *ast_arena;

    char user[];
} scan_context_t;

CT_BEGIN_API

CT_BROKER_API logger_t *ctx_get_logger(const scan_t *scan);
CT_BROKER_API arena_t *ctx_get_arena(const scan_t *scan);
CT_BROKER_API arena_t *ctx_get_string_arena(const scan_t *scan);
CT_BROKER_API arena_t *ctx_get_ast_arena(const scan_t *scan);

CT_BROKER_API void *ctx_get_user(const scan_t *scan);

CT_BROKER_API void ctx_error(const where_t *where, const void *state, const scan_t *scan, const char *msg);
CT_BROKER_API void ctx_unknown_symbol(const scan_t *scan, const where_t *where, const char *msg);

CT_END_API
