#include "ctu/scan.h"

#include "core/macros.h"

#include "cthulhu/events/events.h"
#include "scan/node.h"

ctu_scan_t *ctu_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}

ctu_digit_t ctu_parse_digit(scan_t *scan, where_t where, const char *str, size_t base)
{
    ctu_scan_t *ctx = ctu_scan_context(scan);

    ctu_digit_t result = {
        .value = 0
    };

    int ret = mpz_init_set_str(result.value, str, (int)base);

    if (ret == -1)
    {
        const node_t *node = node_new(scan, where);
        msg_notify(ctx->reports, &kEvent_InvalidIntegerLiteral, node, "failed to parse base %zu digit '%s'", base, str);
    }

    return result;
}

void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CT_UNUSED(state);

    ctu_scan_t *ctx = ctu_scan_context(scan);

    evt_scan_error(ctx->reports, node_new(scan, *where), msg);
}
