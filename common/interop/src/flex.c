#include "core/macros.h"
#include "base/panic.h"

#include "report/report.h"

#include "interop/flex.h"

#include <limits.h>
#include <string.h>

void flex_action(where_t *where, const char *text)
{
    where->firstLine = where->lastLine;
    where->firstColumn = where->lastColumn;

    for (int64_t i = 0; text[i]; i++)
    {
        if (text[i] == '\n')
        {
            where->lastLine += 1;
            where->lastColumn = 0;
        }
        else
        {
            where->lastColumn += 1;
        }
    }
}

int flex_input(scan_t *scan, char *out, int size)
{
    CTASSERTF(size <= INT_MAX, "flex-input(scan=%s, size=%d > INT_MAX)", scan_path(scan), size);
    return (int)scan_read(scan, out, size);
}

void flex_init(where_t *where)
{
    where->firstLine = 0;
    where->firstColumn = 0;
    where->lastLine = 0;
    where->lastColumn = 0;
}

void flex_update(where_t *where, where_t *offsets, int steps)
{
    if (steps)
    {
        where_t rhs1 = offsets[1];
        where_t rhsn = offsets[steps];

        where->firstLine = rhs1.firstLine;
        where->firstColumn = rhs1.firstColumn;
        where->lastLine = rhsn.lastLine;
        where->lastColumn = rhsn.lastColumn;
    }
    else
    {
        where->lastLine = offsets[0].lastLine;
        where->lastColumn = offsets[0].lastColumn;
    }
}
