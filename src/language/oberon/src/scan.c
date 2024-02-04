#include "oberon/scan.h"

#include "core/macros.h"

#include "cthulhu/events/events.h"

obr_scan_t *obr_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}

void obrerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CT_UNUSED(state);

    obr_scan_t *ctx = obr_scan_context(scan);

    evt_scan_error(ctx->logger, node_new(scan, *where), msg);
}
