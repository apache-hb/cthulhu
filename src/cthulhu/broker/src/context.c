#include "cthulhu/broker/scan.h"

#include "cthulhu/events/events.h"

#include "scan/node.h"
#include "scan/scan.h"

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