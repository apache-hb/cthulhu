#include "base/panic.h"

#include "scan/scan.h"

#include "interop/flex.h"

#include <limits.h>

USE_DECL
void flex_action(where_t *where, const char *text)
{
    CTASSERT(where != NULL);
    CTASSERT(text != NULL);

    where->first_line = where->last_line;
    where->first_column = where->last_column;

    for (int i = 0; text[i]; i++)
    {
        if (text[i] == '\n')
        {
            where->last_line += 1;
            where->last_column = 0;
        }
        else
        {
            where->last_column += 1;
        }
    }
}

USE_DECL
int flex_input(scan_t *scan, char *out, int size)
{
    CTASSERT(scan != NULL);
    CTASSERT(out != NULL);

    CTASSERTF(size <= INT_MAX, "flex_input() size is too large (scan=%s, size=%d)", scan_path(scan), size);
    return (int)scan_read(scan, out, size);
}

USE_DECL
void flex_init(where_t *where)
{
    CTASSERT(where != NULL);

    where->first_line = 0;
    where->first_column = 0;
    where->last_line = 0;
    where->last_column = 0;
}

USE_DECL
void flex_update(where_t *where, const where_t *offsets, int steps)
{
    CTASSERT(where != NULL);
    CTASSERT(offsets != NULL);

    if (steps)
    {
        where_t rhs1 = offsets[1];
        where_t rhsn = offsets[steps];

        where->first_line = rhs1.first_line;
        where->first_column = rhs1.first_column;
        where->last_line = rhsn.last_line;
        where->last_column = rhsn.last_column;
    }
    else
    {
        where->last_line = offsets[0].last_line;
        where->last_column = offsets[0].last_column;
    }
}
