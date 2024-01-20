#include "ref/scan.h"

#include "core/macros.h"
#include "cthulhu/events/events.h"
#include "std/typed/vector.h"

ref_scan_t *ref_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}

void ref_parse_digit(mpz_t digit, scan_t *scan, where_t where, const char *str, size_t base)
{
    ref_scan_t *ctx = ref_scan_context(scan);

    int ret = mpz_init_set_str(digit, str, (int)base);

    if (ret == -1)
    {
        const node_t *node = node_new(scan, where);
        msg_notify(ctx->reports, &kEvent_InvalidIntegerLiteral, node, "failed to parse base %zu digit '%s'", base, str);
    }
}

typevec_t *stringlist_begin(scan_t *scan, where_t where, text_t text)
{
    CTU_UNUSED(where);

    arena_t *arena = scan_get_arena(scan);

    typevec_t *list = typevec_new(sizeof(char), text.length, arena);
    return stringlist_append(list, text);
}

typevec_t *stringlist_append(typevec_t *list, text_t text)
{
    typevec_append(list, text.text, text.length);
    return list;
}

void referror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CTU_UNUSED(state);

    ref_scan_t *ctx = ref_scan_context(scan);

    evt_scan_error(ctx->reports, node_new(scan, *where), msg);
}
