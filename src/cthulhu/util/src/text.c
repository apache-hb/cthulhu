// SPDX-License-Identifier: LGPL-3.0-only

#include "cthulhu/util/text.h"

#include "base/panic.h"
#include "base/util.h"
#include "cthulhu/events/events.h"
#include "arena/arena.h"
#include "std/typed/vector.h"

#include "notify/notify.h"

typedef struct escape_t
{
    size_t length;
    char code;
} escape_t;

static escape_t escape_new(size_t length, char code)
{
    escape_t escape = {
        .length = length,
        .code = code
    };

    return escape;
}

static escape_t consume_escape(logger_t *reports, const node_t *node, const char *text)
{
    switch (*text)
    {
    case 'n': return escape_new(2, '\n');
    case 't': return escape_new(2, '\t');
    case 'r': return escape_new(2, '\r');
    case '0': return escape_new(2, '\0');
    case '\\': return escape_new(2, '\\');
    case '"': return escape_new(2, '"');
    case '\'': return escape_new(2, '\'');

    default:
        msg_notify(reports, &kEvent_InvalidStringEscape, node, "unknown escape sequence '\\%c'", *text);
        return escape_new(1, *text);
    }
}

static escape_t consume_text(logger_t *reports, const node_t *node, const char *text)
{
    switch (*text)
    {
    case '\\': return consume_escape(reports, node, text + 1);
    default: return escape_new(1, *text);
    }
}

STA_DECL
text_t util_text_escape(logger_t *reports, const node_t *node, const char *text, size_t length, arena_t *arena)
{
    CTASSERT(reports != NULL);
    CTASSERT(node != NULL);
    CTASSERT(text != NULL);
    CTASSERT(arena != NULL);

    typevec_t vec = typevec_make(sizeof(char), length, arena);
    ARENA_IDENTIFY(typevec_data(&vec), "text", reports, arena);

    for (size_t i = 0; i < length;)
    {
        escape_t escape = consume_text(reports, node, text + i);
        typevec_push(&vec, &escape.code);
        i += escape.length;
    }

    char zero = '\0';
    typevec_push(&vec, &zero);

    return text_make(typevec_data(&vec), typevec_len(&vec));
}

STA_DECL
bool util_text_has_escapes(const char *text, size_t length)
{
    CTASSERT(text != NULL);

    for (size_t i = 0; i < length; i++)
    {
        if (text[i] == '\\')
        {
            return true;
        }
    }

    return false;
}
