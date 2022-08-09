#include "suffix.h"

#include "base/memory.h"
#include "base/panic.h"

#include "cthulhu/hlir/hlir.h"

#include "lang/sema.h"

typedef struct
{
    digit_t width;
    sign_t sign;
} int_type_t;

static int_type_t *new_int_type(digit_t digit, sign_t sign)
{
    int_type_t *it = ctu_malloc(sizeof(int_type_t));
    it->width = digit;
    it->sign = sign;
    return it;
}

static suffix_t *new_suffix(astof_t expected, apply_suffix_t apply, void *data)
{
    CTASSERT(apply != NULL);

    suffix_t *suffix = ctu_malloc(sizeof(suffix_t));
    suffix->expected = expected;
    suffix->apply = apply;
    suffix->data = data;
    return suffix;
}

static hlir_t *apply_int_suffix(sema_t *sema, ast_t *ast, suffix_t *suffix)
{
    UNUSED(sema);

    int_type_t *type = suffix->data;
    hlir_t *kind = get_digit_type(type->sign, type->width);
    return hlir_digit_literal(ast->node, kind, ast->digit);
}

static void add_suffix(sema_t *sema, const char *name, suffix_t *suffix)
{
    sema_set(sema, eTagSuffix, name, suffix);
}

static void add_int_suffix(sema_t *sema, const char *suffix, digit_t digit, sign_t sign)
{
    add_suffix(sema, suffix, new_suffix(eAstDigit, apply_int_suffix, new_int_type(digit, sign)));
}

void add_builtin_suffixes(sema_t *sema)
{
    add_int_suffix(sema, "", eDigitInt, eSigned);
    add_int_suffix(sema, "l", eDigitLong, eSigned);

    add_int_suffix(sema, "u", eDigitInt, eUnsigned);
    add_int_suffix(sema, "ul", eDigitLong, eUnsigned);
    add_int_suffix(sema, "uz", eDigitSize, eUnsigned);

    add_int_suffix(sema, "p", eDigitPtr, eUnsigned);
}

hlir_t *apply_suffix(sema_t *sema, ast_t *ast, suffix_t *suffix)
{
    if (ast->of != suffix->expected)
    {
        report(sema_reports(sema), eFatal, ast->node, "incorrect literal for suffix");
        return hlir_error(ast->node, "invalid suffix application");
    }

    return suffix->apply(sema, ast, suffix);
}
