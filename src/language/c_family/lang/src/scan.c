#include "c/scan.h"

#include "base/panic.h"
#include "cthulhu/events/events.h"
#include "scan/node.h"

#include "core/macros.h"

cc_scan_t *cc_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}

c_ast_t *cc_get_typedef_name(cc_scan_t *scan, const char *name)
{
    CTASSERT(scan != NULL);
    CTASSERT(name != NULL);

    return NULL;
}

void ccerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CT_UNUSED(state);

    cc_scan_t *ctx = cc_scan_context(scan);

    evt_scan_error(ctx->logger, node_new(scan, *where), msg);
}

void cc_on_error(scan_t *scan, const char *msg, where_t where)
{
    cc_scan_t *ctx = cc_scan_context(scan);

    msg_notify(ctx->logger, &kEvent_SyntaxError, node_new(scan, where), "%s", msg);
}
