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
} lex_extra_t;

void ctuerror(where_t *where, void *state, scan_t scan, const char *msg)
{
    UNUSED(state);

    report(scan_reports(scan), eFatal, node_new(scan, *where), "%s", msg);
}

void init_scan(scan_t scan)
{
    lex_extra_t *extra = ctu_malloc(sizeof(lex_extra_t));
    extra->depth = 0;
    extra->attribs = vector_new(4);
    scan_set(scan, extra);
}

void enter_template(scan_t scan)
{
    lex_extra_t *extra = scan_get(scan);
    extra->depth++;
}

size_t exit_template(scan_t scan)
{
    lex_extra_t *extra = scan_get(scan);
    return extra->depth--;
}

void add_attribute(scan_t scan, ast_t *ast)
{
    lex_extra_t *extra = scan_get(scan);
    vector_push(&extra->attribs, ast);
}

vector_t *collect_attributes(scan_t scan)
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
