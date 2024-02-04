#include "pl0/scan.h"

#include "core/macros.h"
#include "cthulhu/events/events.h"

pl0_scan_t *pl0_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}

void pl0error(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CT_UNUSED(state);

    pl0_scan_t *ctx = pl0_scan_context(scan);

    evt_scan_error(ctx->logger, node_new(scan, *where), msg);
}
