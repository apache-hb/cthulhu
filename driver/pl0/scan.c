#include "scan.h"

#include "base/macros.h"
#include "base/util.h"
#include "report/report.h"
#include "std/str.h"

char *pl0_normalize(const char *ident)
{
    return str_lower(ident);
}

void pl0error(where_t *where, void *state, scan_t *scan, const char *msg)
{
    UNUSED(state);

    report(scan_reports(scan), eFatal, node_new(scan, *where), "%s", msg);
}
