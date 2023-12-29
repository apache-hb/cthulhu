#include "base/panic.h"

#include "interop/flex.h"

#include <limits.h>

void flex_action(where_t *where, const char *text)
{
    where->first_line = where->last_line;
    where->first_column = where->last_column;

    for (int64_t i = 0; text[i]; i++)
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

int flex_input(scan_t *scan, char *out, int size)
{
    CTASSERTF(size <= INT_MAX, "flex-input(scan=%s, size=%d > INT_MAX)", scan_path(scan), size);
    return (int)scan_read(scan, out, size);
}

void flex_init(where_t *where)
{
    where->first_line = 0;
    where->first_column = 0;
    where->last_line = 0;
    where->last_column = 0;
}

void flex_update(where_t *where, where_t *offsets, int steps)
{
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
