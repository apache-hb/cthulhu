#include "pl0/scan.h"

#include "base/macros.h"
#include "base/util.h"
#include "report/report.h"
#include "std/str.h"

void pl0error(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CTU_UNUSED(state);

    report(scan_reports(scan), eFatal, node_new(scan, *where), "%s", msg);
}
