#include "cc/scan.h"

#include "cthulhu/events/events.h"
#include "scan/node.h"

#include "core/macros.h"

cc_scan_t *cc_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}

void ccerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CTU_UNUSED(state);

    cc_scan_t *ctx = cc_scan_context(scan);

    evt_scan_error(ctx->reports, node_new(scan, *where), msg);
}
