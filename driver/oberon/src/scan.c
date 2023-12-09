#include "scan/node.h"

#include "report/report.h"

#include "core/macros.h"

void obrerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CTU_UNUSED(state);

    report(scan_reports(scan), eFatal, node_new(scan, *where), "%s", msg);
}
