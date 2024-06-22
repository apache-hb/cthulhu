#include "meta/scan.h"

#include "core/macros.h"

#include "cthulhu/events/events.h"

meta_scan_t *meta_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}

void metaerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CT_UNUSED(state);

    meta_scan_t *ctx = meta_scan_context(scan);

    evt_scan_error(ctx->logger, node_new(scan, *where), msg);
}
