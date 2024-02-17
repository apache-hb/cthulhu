#include "c/scan.h"

#include "base/panic.h"
#include "cthulhu/broker/scan.h"
#include "cthulhu/events/events.h"
#include "scan/node.h"

c_ast_t *cc_get_typedef_name(scan_t *scan, const char *name)
{
    CTASSERT(scan != NULL);
    CTASSERT(name != NULL);

    return NULL;
}

void ccerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    ctx_error(where, state, scan, msg);
}

void cc_on_error(scan_t *scan, const char *msg, where_t where)
{
    logger_t *logger = ctx_get_logger(scan);
    msg_notify(logger, &kEvent_SyntaxError, node_new(scan, where), "%s", msg);
}
