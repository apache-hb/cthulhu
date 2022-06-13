#include "gen/gen.h"

#include "base/macros.h"
#include "report/report.h"

void generror(where_t *where, void *state, scan_t scan, const char *msg)
{
    UNUSED(state);

    report(scan_reports(scan), ERROR, node_new(scan, *where), "%s", msg);
}
