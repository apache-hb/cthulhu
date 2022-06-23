#include "ast.gen.h"
#include "base/macros.h"
#include "report/report.h"

void pl0error(where_t *where, void *state, scan_t scan, const char *msg)
{
	UNUSED(state);
	report(scan_reports(scan), ERROR, node_new(scan, *where), "%s", msg);
}
