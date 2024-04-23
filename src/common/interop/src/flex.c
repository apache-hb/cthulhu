// SPDX-License-Identifier: LGPL-3.0-only

#include "interop/actions.h"

#include "base/panic.h"

#include "core/where.h"
#include "scan/scan.h"

#include <limits.h>

STA_DECL
void flex_action(where_t *where, const char *text)
{
    CTASSERT(where != NULL);
    CTASSERT(text != NULL);

    where_t tmp = *where;
    tmp.first_line = tmp.last_line;
    tmp.first_column = tmp.last_column;

    for (int i = 0; text[i]; i++)
    {
        if (text[i] == '\n')
        {
            tmp.last_line += 1;
            tmp.last_column = 0;
        }
        else
        {
            tmp.last_column += 1;
        }
    }

    *where = tmp;
}

STA_DECL
int flex_input(scan_t *scan, char *out, int size)
{
    CTASSERT(scan != NULL);
    CTASSERT(out != NULL);

    CTASSERTF(size <= INT_MAX, "flex_input() size is too large (scan=%s, size=%d)", scan_path(scan), size);
    return (int)scan_read(scan, out, size);
}

STA_DECL
void flex_init(where_t *where)
{
    CTASSERT(where != NULL);

    where_t zero = { 0 };
    *where = zero;
}

STA_DECL
void flex_update(where_t *where, const where_t *offsets, int steps)
{
    CTASSERT(where != NULL);
    CTASSERT(offsets != NULL);

    if (steps)
    {
        where_t rhs1 = offsets[1];
        where_t rhsn = offsets[steps];

        where_t tmp = {
            .first_line = rhs1.first_line,
            .first_column = rhs1.first_column,
            .last_line = rhsn.last_line,
            .last_column = rhsn.last_column,
        };

        *where = tmp;
    }
    else
    {
        where_t rhs = offsets[0];
        where->last_line = rhs.last_line;
        where->last_column = rhs.last_column;
    }
}
