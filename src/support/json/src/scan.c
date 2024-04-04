// SPDX-License-Identifier: LGPL-3.0-only

#include "arena/arena.h"
#include "base/util.h"
#include "json_scan.h"

#include "core/macros.h"

#include "cthulhu/events/events.h"
#include "cthulhu/util/text.h"
#include "std/set.h"

#include <stdlib.h>

json_scan_t *json_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}

void json_parse_integer(mpz_t integer, scan_t *scan, where_t where, const char *text, int base)
{
    int result = mpz_init_set_str(integer, text, base);
    if (result != 0)
    {
        json_scan_t *ctx = json_scan_context(scan);
        msg_notify(ctx->reports, &kEvent_InvalidIntegerLiteral, node_new(scan, where), "invalid integer literal");
    }
}

void json_parse_float(float *real, scan_t *scan, where_t where, const char *text)
{
    // TODO: dont use strtof
    float result = strtof(text, NULL);
    if (result != 1)
    {
        json_scan_t *ctx = json_scan_context(scan);
        msg_notify(ctx->reports, &kEvent_InvalidFloatLiteral, node_new(scan, where), "invalid float literal");
    }
    *real = result;
}

void json_parse_string(text_view_t *string, scan_t *scan, json_where_t where, const char *text, size_t length)
{
    // if we dont have any escapes then we return a view of the source text
    if (!util_text_has_escapes(text, length))
    {
        text_view_t source = scan_source(scan);
        text_view_t span = text_view_make(source.text + where.offset - length - 1, length);
        *string = span;
    }
    else
    {
        // otherwise escape the text
        json_scan_t *ctx = json_scan_context(scan);
        arena_t *arena = scan_get_arena(scan);
        node_t node = node_make(scan, where.where);
        text_t escaped = util_text_escape(ctx->reports, &node, text, length, arena);
        *string = text_view_make(escaped.text, escaped.length);
    }
}

void jsonerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CT_UNUSED(state);

    json_scan_t *ctx = json_scan_context(scan);

    evt_scan_error(ctx->reports, node_new(scan, *where), msg);
}
