#include "pl0/scan.h"

#include "core/macros.h"
#include "report/report.h"

void pl0error(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CTU_UNUSED(state);

    report(scan_reports(scan), eFatal, node_new(scan, *where), "%s", msg);
}
