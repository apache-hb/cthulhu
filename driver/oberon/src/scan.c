#include "scan.h"

#include "base/macros.h"
#include "report/report.h"

void obrerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CTU_UNUSED(state);

    report(scan_reports(scan), eFatal, node_new(scan, *where), "%s", msg);
}
