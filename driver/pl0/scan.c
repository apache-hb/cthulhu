#include "scan.h"

#include "base/macros.h"
#include "base/util.h"
#include "report/report.h"

#include <limits.h>

static char safe_tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return (c + 'a' - 'A') & CHAR_MAX;
    }

    if (CHAR_MIN <= c && c <= CHAR_MAX)
    {
        return c;
    }

    return '\0';
}

char *pl0_normalize(const char *ident)
{
    char *out = ctu_strdup(ident);
    for (char *p = out; *p; ++p)
    {
        *p = safe_tolower(*p);
    }
    return out;
}

void pl0error(where_t *where, void *state, scan_t scan, const char *msg)
{
    UNUSED(state);

    report(scan_reports(scan), ERROR, node_new(scan, *where), "%s", msg);
}
