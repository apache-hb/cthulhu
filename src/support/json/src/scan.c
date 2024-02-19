#include "json_scan.h"

#include "core/macros.h"

#include "cthulhu/events/events.h"
#include "cthulhu/util/text.h"

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
    float result = strtof(text, NULL);
    if (result != 1)
    {
        json_scan_t *ctx = json_scan_context(scan);
        msg_notify(ctx->reports, &kEvent_InvalidFloatLiteral, node_new(scan, where), "invalid float literal");
    }
    *real = result;
}

void json_parse_string(text_t *string, scan_t *scan, where_t where, const char *text, size_t length)
{
    json_scan_t *ctx = json_scan_context(scan);
    arena_t *arena = scan_get_arena(scan);
    *string = util_text_escape(ctx->reports, node_new(scan, where), text, length, arena);
}

void jsonerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CT_UNUSED(state);

    json_scan_t *ctx = json_scan_context(scan);

    evt_scan_error(ctx->reports, node_new(scan, *where), msg);
}
