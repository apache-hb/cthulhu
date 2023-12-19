#include "oberon/scan.h"

#include "core/macros.h"

obr_scan_t *obr_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}

void obrerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CTU_UNUSED(state);

    obr_scan_t *ctx = obr_scan_context(scan);

    report(ctx->reports, eFatal, node_new(scan, *where), "%s", msg);
}
