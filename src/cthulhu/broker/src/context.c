#include "cthulhu/broker/scan.h"

#include "scan/scan.h"

logger_t *get_logger(const scan_t *scan)
{
    scan_context_t *ctx = scan_get_context(scan);
    return ctx->logger;
}

arena_t *get_arena(const scan_t *scan)
{
    scan_context_t *ctx = scan_get_context(scan);
    return ctx->arena;
}

arena_t *get_string_arena(const scan_t *scan)
{
    scan_context_t *ctx = scan_get_context(scan);
    return ctx->string_arena;
}

arena_t *get_ast_arena(const scan_t *scan)
{
    scan_context_t *ctx = scan_get_context(scan);
    return ctx->ast_arena;
}

void *get_user(const scan_t *scan)
{
    scan_context_t *ctx = scan_get_context(scan);
    return ctx->user;
}
