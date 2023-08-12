#include "suffix.h"

#include "base/memory.h"
#include "base/panic.h"

#include "cthulhu/hlir/h2.h"

#include "sema/sema.h"

#include "report/report.h"

typedef struct
{
    digit_t width;
    sign_t sign;
} int_type_t;

static int_type_t *int_type_new(digit_t digit, sign_t sign)
{
    int_type_t *it = ctu_malloc(sizeof(int_type_t));
    it->width = digit;
    it->sign = sign;
    return it;
}

static suffix_t *suffix_new(astof_t expected, apply_suffix_t apply, void *data)
{
    CTASSERT(apply != NULL);

    suffix_t *suffix = ctu_malloc(sizeof(suffix_t));
    suffix->expected = expected;
    suffix->apply = apply;
    suffix->data = data;
    return suffix;
}

static h2_t *apply_int_suffix(h2_t *sema, ast_t *ast, suffix_t *suffix)
{
    CTU_UNUSED(sema);

    int_type_t *type = suffix->data;
    h2_t *kind = get_digit_type(type->sign, type->width);
    return h2_expr_digit(ast->node, kind, ast->digit);
}

static void add_suffix(h2_t *sema, const char *name, suffix_t *suffix)
{
    h2_t *prev = h2_module_get(sema, eTagSuffix, name);
    CTASSERTF(prev == NULL, "suffix '%s' already exists", name);

    h2_module_set(sema, eTagSuffix, name, suffix);
}

static void add_int_suffix(h2_t *sema, const char *suffix, digit_t digit, sign_t sign)
{
    add_suffix(sema, suffix, suffix_new(eAstDigit, apply_int_suffix, int_type_new(digit, sign)));
}

void add_builtin_suffixes(h2_t *sema)
{
    add_int_suffix(sema, "", eDigitInt, eSignSigned);
    add_int_suffix(sema, "l", eDigitLong, eSignSigned);

    add_int_suffix(sema, "u", eDigitInt, eSignUnsigned);
    add_int_suffix(sema, "ul", eDigitLong, eSignUnsigned);
    add_int_suffix(sema, "uz", eDigitSize, eSignUnsigned);

    add_int_suffix(sema, "p", eDigitPtr, eSignUnsigned);
}

h2_t *apply_suffix(h2_t *sema, ast_t *ast, suffix_t *suffix)
{
    if (ast->of != suffix->expected)
    {
        report(sema->reports, eFatal, ast->node, "incorrect literal for suffix");
        return h2_error(ast->node, "invalid suffix application");
    }

    return suffix->apply(sema, ast, suffix);
}
