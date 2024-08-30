// SPDX-License-Identifier: LGPL-3.0-only
#include "base/panic.h"
#include "json_actions.h"

#include "interop/actions.h"

void json_action(json_where_t *where, const char *text, size_t length)
{
    flex_action(&where->where, text);
    where->offset += length;
}

void json_init(json_where_t *where)
{
    flex_init(&where->where);
    where->offset = 0;
}

// TODO: some of this could be deduplicated with flex_update
void json_update(json_where_t *where, const json_where_t *offsets, int steps)
{
    CTASSERT(where != NULL);
    CTASSERT(offsets != NULL);

    if (steps)
    {
        json_where_t rhs1 = offsets[1];
        json_where_t rhsn = offsets[steps];

        where_t tmp = {
            .first_line = rhs1.where.first_line,
            .first_column = rhs1.where.first_column,
            .last_line = rhsn.where.last_line,
            .last_column = rhsn.where.last_column,
        };

        where->where = tmp;
    }
    else
    {
        json_where_t rhs = offsets[0];
        where_t tmp = where->where;
        tmp.last_line = rhs.where.last_line;
        tmp.last_column = rhs.where.last_column;

        // TODO: is this correct?
        where->where = tmp;
    }
}
