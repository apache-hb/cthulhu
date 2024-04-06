// SPDX-License-Identifier: LGPL-3.0-or-later
#include "query_scan.h"

#include "core/macros.h"
#include "cthulhu/events/events.h"
#include "cthulhu/util/text.h"

query_scan_t *query_scan_context(scan_t *scan)
{
    return (query_scan_t *)scan_get_context(scan);
}

void query_parse_integer(mpz_t integer, scan_t *scan, where_t where, const char *text, int base)
{
    int result = mpz_init_set_str(integer, text, base);
    if (result != 0)
    {
        query_scan_t *ctx = query_scan_context(scan);
        node_t node = node_make(scan, where);
        msg_notify(ctx->reports, &kEvent_InvalidIntegerLiteral, &node, "invalid integer literal");
    }
}

void query_parse_string(text_t *string, scan_t *scan, where_t where, const char *text, size_t length)
{
    query_scan_t *ctx = query_scan_context(scan);
    arena_t *arena = scan_get_arena(scan);
    node_t node = node_make(scan, where);
    *string = util_text_escape(ctx->reports, &node, text, length, arena);
}

void queryerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CT_UNUSED(state);

    query_scan_t *ctx = query_scan_context(scan);

    node_t node = node_make(scan, *where);
    evt_scan_error(ctx->reports, &node, msg);
}
