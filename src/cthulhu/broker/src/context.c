// SPDX-License-Identifier: LGPL-3.0-only

#include "cthulhu/broker/scan.h"

#include "cthulhu/events/events.h"

#include "scan/node.h"
#include "scan/scan.h"

#include "std/vector.h"

#include "core/macros.h"

logger_t *ctx_get_logger(const scan_t *scan)
{
    scan_context_t *ctx = scan_get_context(scan);
    return ctx->logger;
}

arena_t *ctx_get_arena(const scan_t *scan)
{
    scan_context_t *ctx = scan_get_context(scan);
    return ctx->arena;
}

arena_t *ctx_get_string_arena(const scan_t *scan)
{
    scan_context_t *ctx = scan_get_context(scan);
    return ctx->string_arena;
}

arena_t *ctx_get_ast_arena(const scan_t *scan)
{
    scan_context_t *ctx = scan_get_context(scan);
    return ctx->ast_arena;
}

vector_t *ctx_vector_init(void *init, const scan_t *scan)
{
    return vector_init(init, ctx_get_arena(scan));
}

vector_t *ctx_vector_new(size_t size, const scan_t *scan)
{
    return vector_new(size, ctx_get_arena(scan));
}

vector_t *ctx_vector_of(size_t size, const scan_t *scan)
{
    return vector_of(size, ctx_get_arena(scan));
}

void *ctx_get_user(const scan_t *scan)
{
    scan_context_t *ctx = scan_get_context(scan);
    return ctx->user;
}

void ctx_error(const where_t *where, const void *state, const scan_t *scan, const char *msg)
{
    CT_UNUSED(state);

    logger_t *logger = ctx_get_logger(scan);
    node_t *node = node_new(scan, *where);

    evt_scan_error(logger, node, msg);
}

void ctx_unknown_symbol(const scan_t *scan, const where_t *where, const char *msg)
{
    logger_t *logger = ctx_get_logger(scan);
    node_t *node = node_new(scan, *where);

    evt_scan_unknown(logger, node, msg);
}
