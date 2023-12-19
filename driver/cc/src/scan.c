#include "cc/scan.h"
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

    report(ctx->reports, eFatal, node_new(scan, *where), "%s", msg);
}
