#include "pl0/scan.h"

#include "core/macros.h"

pl0_scan_t *pl0_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}

void pl0error(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CTU_UNUSED(state);

    pl0_scan_t *ctx = pl0_scan_context(scan);

    report(ctx->reports, eFatal, node_new(scan, *where), "%s", msg);
}
