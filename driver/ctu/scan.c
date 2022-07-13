#include "scan.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"

#include "report/report.h"

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

static digit_t trim_width(char *text)
{
    size_t len = strlen(text);
    CTASSERTM(len > 0, "trim-width got empty string");

    digit_t result = eInt;
    switch (text[len - 1])
    {
    case 'c':
    case 'C':
        result = eChar;
        break;

    case 's':
    case 'S':
        result = eShort;
        break;

    case 'l':
    case 'L':
        result = eLong;
        break;

    case 'z':
    case 'Z':
        result = eIntSize;
        break;

    case 'p':
    case 'P':
        result = eIntPtr;
        break;

    case 'm':
    case 'M':
        result = eIntMax;
        break;

    default:
        return eInt;
    }

    text[len - 1] = '\0';
    return result;
}

static sign_t trim_sign(char *text)
{
    size_t len = strlen(text);
    CTASSERTM(len > 0, "trim-sign passed empty string");

    switch (text[len - 1])
    {
    case 'u':
    case 'U':
        text[len - 1] = '\0';
        return eUnsigned;
    default:
        return eSigned;
    }
}

void init_string_with_suffix(suffix_t *suffix, mpz_t mpz, char *text, size_t base)
{
    suffix_t out = {eInt, eSigned};
    size_t len = strlen(text);
    if (len == 0)
    {
        mpz_init_set_d(mpz, 0);
        *suffix = out;
        return;
    }

    out.digit = trim_width(text);
    out.sign = trim_sign(text);

    mpz_init_set_str(mpz, text, base);

    *suffix = out;
}
