#include "cthulhu/util/text.h"

#include "report/report.h"

#include "std/typed/vector.h"

typedef struct escape_t {
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

static escape_t consume_escape(reports_t *reports, const node_t *node, const char *text)
{
    switch (*text)
    {
    case 'n': return escape_new(2, '\n');
    case 't': return escape_new(2, '\t');
    case 'r': return escape_new(2, '\r');
    case '0': return escape_new(2, '\0');
    case '\\': return escape_new(2, '\\');

    default:
        report(reports, eWarn, node, "unknown escape sequence '\\%c'", *text);
        return escape_new(1, *text);
    }
}

static escape_t consume_text(reports_t *reports, const node_t *node, const char *text)
{
    switch (*text)
    {
    case '\\': return consume_escape(reports, node, text + 1);
    default: return escape_new(1, *text);
    }
}

util_text_t util_text_escape(reports_t *reports, const node_t *node, const char *text, size_t length)
{
    typevec_t *vec = typevec_new(sizeof(char), length);

    for (size_t i = 0; i < length;)
    {
        escape_t escape = consume_text(reports, node, text + i);
        typevec_push(vec, &escape.code);
        i += escape.length;
    }

    char zero = '\0';
    typevec_push(vec, &zero);

    util_text_t result = {
        .text = typevec_data(vec),
        .length = typevec_len(vec)
    };

    return result;
}
