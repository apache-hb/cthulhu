#include "scan.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"
#include "base/util.h"

#include "report/report.h"

#include "std/str.h"
#include "std/vector.h"

#include <string.h>

typedef struct
{
    size_t depth;      // template depth
    vector_t *attribs; // attribute collector
    driver_t *handle;
} lex_extra_t;

void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    UNUSED(state);

    report(scan_reports(scan), eFatal, node_new(scan, *where), "%s", msg);
}

void ctu_init_scan(scan_t *scan, driver_t *handle)
{
    lex_extra_t *extra = ctu_malloc(sizeof(lex_extra_t));
    extra->depth = 0;
    extra->attribs = vector_new(4);
    extra->handle = handle;
    scan_set(scan, extra);
}

void enter_template(scan_t *scan)
{
    lex_extra_t *extra = scan_get(scan);
    extra->depth++;
}

size_t exit_template(scan_t *scan)
{
    lex_extra_t *extra = scan_get(scan);
    return extra->depth--;
}

driver_t *get_lang_handle(scan_t *scan)
{
    lex_extra_t *extra = scan_get(scan);
    return extra->handle;
}

void add_attribute(scan_t *scan, ast_t *ast)
{
    lex_extra_t *extra = scan_get(scan);
    vector_push(&extra->attribs, ast);
}

vector_t *collect_attributes(scan_t *scan)
{
    lex_extra_t *extra = scan_get(scan);
    vector_t *vec = extra->attribs;
    extra->attribs = vector_new(4);
    return vec;
}

static const char *kDigits = "0123456789abcdef";

static bool is_digit_in_base(char c, size_t base)
{
    CTASSERT(base <= 16);

    char lower = str_tolower(c);
    for (size_t i = 0; i < base; i++)
    {
        if (kDigits[i] == lower)
        {
            return true;
        }
    }

    return false;
}

char *init_string_with_suffix(mpz_t mpz, const char *text, int base)
{
    size_t len = strlen(text);
    CTASSERT(len > 0);

    size_t i = 0;

    for (; i < len; i++)
    {
        if (!is_digit_in_base(text[i], base))
        {
            break;
        }
    }

    // get the suffix and init mpz with the actual digit
    char *suffix = str_lower(text + i);
    char *digit = ctu_strndup(text, i);
    mpz_init_set_str(mpz, digit, base);

    return suffix;
}

static char parse_escape(const char *text)
{
    char c = text[0];
    switch (c) {
    case 'a': return '\a';
    case 'b': return '\b';
    case 'f': return '\f';
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    case 'v': return '\v';
    case '\\': return '\\';
    case '\'': return '\'';
    case '"': return '"';
    case '?': return '\?';
    default: return c;
    }
}

string_t parse_string_escapes(reports_t *reports, const char *text, size_t len)
{
    char *str = ctu_malloc(len + 1);
    size_t size = 0;
    bool escaped = false;

    for (; size < len;)
    {
        char c = text[size];
        if (c == '\\')
        {
            escaped = true;
            size_t required = size + 1;
            if (required >= len)
            {
                report(reports, eWarn, NULL, "string literal has trailing backslash");
                break;
            }

            str[size] = parse_escape(text + size + 1);
            size = required;
        }
        else
        {
            escaped = false;
            str[size] = c;
        }

        size += 1;
    }

    size_t offset = escaped ? 1 : 0;

    string_t result = {
        .text = str,
        .size = size - offset // TODO: this seems wrong
    };

    return result;
}

void init_decimal(mpq_t result, const char *text)
{
    vector_t *parts = str_split(text, ".");
    CTASSERT(vector_len(parts) == 2);
    const char *num = vector_get(parts, 0);
    const char *den = vector_get(parts, 1);
    
    mpz_t numq;
    mpz_init_set_str(numq, num, 10);

    mpz_t denq;
    mpz_init_set_str(denq, den, 10);
    
    mpq_init(result);
    mpq_set_num(result, numq);
    mpq_set_den(result, denq);
}
