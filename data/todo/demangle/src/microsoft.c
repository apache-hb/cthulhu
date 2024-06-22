// SPDX-License-Identifier: LGPL-3.0-or-later
#include "demangle/microsoft.h"

#include "base/panic.h"
#include "core/macros.h"

#include "std/typed/vector.h"
#include <stdio.h>

typedef struct ms_demangle_t {
    arena_t *arena;
    const char *front;
    const char *back;
    const char *cursor;

    typevec_t *nodes;
    typevec_t *errors;
} ms_demangle_t;

///
/// range handling and helpers
///

static buffer_range_t msd_invalid_range(void)
{
    buffer_range_t range = { UINT_MAX, UINT_MAX };
    return range;
}

static bool msd_range_is_valid(buffer_range_t range)
{
    return range.front != UINT_MAX && range.back != UINT_MAX;
}

static const char *msd_range_text(const ms_demangle_t *self, buffer_range_t range)
{
    return self->front + range.front;
}

static int msd_range_length(buffer_range_t range)
{
    return range.back - range.front;
}

///
/// lexing functions
///

static bool msd_is_empty(const ms_demangle_t *self)
{
    return self->cursor == self->back;
}

static bool msd_consume_char(ms_demangle_t *self, char c)
{
    if (msd_is_empty(self) || *self->cursor != c)
        return false;

    self->cursor++;

    return true;
}

static void msd_expect_char(ms_demangle_t *self, char c)
{
    CTASSERTF_ALWAYS(msd_consume_char(self, c), "expected '%c' but got '%c'", c, *self->cursor);
}

static ms_buffer_offset_t msd_current_offset(const ms_demangle_t *self)
{
    return (ms_buffer_offset_t)(self->cursor - self->front);
}

///
/// demangler impl
///

static buffer_range_t msd_simple_string(ms_demangle_t *self)
{
    ms_buffer_offset_t start = msd_current_offset(self);

    bool had_tail = false;
    // consume text until we hit a '@'
    while (!msd_is_empty(self))
    {
        if (*self->cursor != '@')
            self->cursor++;
        else
            had_tail = true;
    }

    ms_buffer_offset_t end = msd_current_offset(self);

    if (!had_tail)
    {
        // invalid in some way
        return msd_invalid_range();
    }

    if (start == end)
    {
        // empty string
        return msd_invalid_range();
    }

    msd_expect_char(self, '@');

    buffer_range_t range = { .front = start, .back = end };
    return range;
}

static ms_node_index_t msd_qualified_name(ms_demangle_t *self)
{
    // demangle a list of id@id@id
    buffer_range_t range = msd_invalid_range();
    do
    {
        printf("simple: %.*s\n", msd_range_length(range), msd_range_text(self, range));
    } while (msd_range_is_valid(range));

    return MS_DEMANGLE_INVALID;
}

ms_node_index_t ms_demangle(text_view_t text, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(text.text != NULL);

    CT_ASSERT_RANGE_PRI(text.length, (size_t)0, (size_t)UINT_MAX, "%zu");

    ms_demangle_t demangle = {
        .arena = arena,
        .front = text.text,
        .back = text.text + text.length,
        .cursor = text.text,
        .nodes = typevec_new(sizeof(ms_demangle_node_t), 16, arena),
        .errors = typevec_new(sizeof(ms_demangle_node_t), 16, arena),
    };

    if (!msd_consume_char(&demangle, '?'))
        return MS_DEMANGLE_INVALID;

    return msd_qualified_name(&demangle);
}
