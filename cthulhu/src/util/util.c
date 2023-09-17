#include "cthulhu/util/type.h"
#include "cthulhu/util/util.h"

#include "cthulhu/tree/query.h"
#include "cthulhu/tree/sema.h"

#include "report/report.h"

#include "scan/node.h"

#include "std/str.h"

#include "base/panic.h"

#include <stdlib.h>

void *util_select_decl(tree_t *sema, const size_t *tags, size_t len, const char *name)
{
    CTASSERTF(tags != NULL && len > 0, "(tags=%p, len=%zu)", tags, len);

    for (size_t i = 0; i < len; i++)
    {
        tree_t *decl = tree_module_get(sema, tags[i], name);
        if (decl != NULL) { return decl; }
    }

    return NULL;
}

static const char *kCurrentModule = "util:current-module";
static const char *kCurrentSymbol = "util:current-symbol";

tree_t *util_current_module(tree_t *sema)
{
    tree_t *current = tree_get_extra(sema, kCurrentModule);
    CTASSERT(current != NULL);
    return current;
}

void util_set_current_module(tree_t *sema, tree_t *module)
{
    CTASSERTF(module != NULL, "(module=%p)", module);
    tree_set_extra(sema, kCurrentModule, module);
}


tree_t *util_current_symbol(tree_t *sema)
{
    tree_t *current = tree_get_extra(sema, kCurrentSymbol);
    CTASSERT(current != NULL);
    return current;
}

void util_set_current_symbol(tree_t *sema, tree_t *symbol)
{
    CTASSERTF(symbol != NULL, "(symbol=%p)", symbol);
    tree_set_extra(sema, kCurrentSymbol, symbol);
}

bool util_types_equal(const tree_t *lhs, const tree_t *rhs)
{
    CTASSERTF(lhs != NULL && rhs != NULL, "(lhs=%p, rhs=%p)", lhs, rhs);

    if (lhs == rhs) { return true; }

    tree_kind_t lhsKind = tree_get_kind(lhs);
    tree_kind_t rhsKind = tree_get_kind(rhs);

    if (lhsKind != rhsKind) { return false; }

    switch (lhsKind)
    {
    case eTreeTypeEmpty:
    case eTreeTypeUnit:
    case eTreeTypeBool:
        return true;

    case eTreeTypeDigit:
        return (lhs->digit == rhs->digit) && (lhs->sign == rhs->sign);

    case eTreeTypeReference:
    case eTreeTypePointer:
        return util_types_equal(lhs->ptr, rhs->ptr);

    default:
        return false;
    }
}

bool util_types_comparable(const tree_t *lhs, const tree_t *rhs)
{
    if (util_types_equal(lhs, rhs)) { return true; }

    tree_kind_t lhsKind = tree_get_kind(lhs);
    tree_kind_t rhsKind = tree_get_kind(rhs);

    if (lhsKind != rhsKind) { return false; }

    switch (lhsKind)
    {
    case eTreeTypeBool:
    case eTreeTypeDigit:
        return true;

    default:
        return false; /* TODO probably wrong */
    }
}

static bool can_cast_length(size_t dst, size_t src)
{
    if (!util_length_bounded(dst)) { return true; }
    if (!util_length_bounded(src)) { return true; }

    return dst < src;
}

static tree_t *cast_check_length(const tree_t *dst, tree_t *expr, size_t dstlen, size_t srclen)
{
    if (!can_cast_length(dstlen, srclen))
    {
        return tree_error(tree_get_node(expr),
                          "creating an array with a length greater than its backing memory is unwise. (%s < %s)",
                          util_length_name(srclen),
                          util_length_name(dstlen));
    }

    return tree_expr_cast(tree_get_node(expr), dst, expr);
}

static tree_t *cast_to_opaque(const tree_t *dst, tree_t *expr)
{
    CTASSERTF(tree_is(dst, eTreeTypeOpaque), "(dst=%s)", tree_to_string(dst));

    const tree_t *src = tree_get_type(expr);

    switch (tree_get_kind(src))
    {
    case eTreeTypePointer:
    case eTreeTypeDigit:
        return tree_expr_cast(tree_get_node(expr), dst, expr); // TODO: a little iffy

    default: return tree_error(tree_get_node(expr),
                                "cannot cast `%s` to `%s`",
                                tree_to_string(src), tree_to_string(dst));
    }
}

static tree_t *cast_to_pointer(const tree_t *dst, tree_t *expr)
{
    CTASSERTF(tree_is(dst, eTreeTypePointer), "(dst=%s)", tree_to_string(dst));

    const tree_t *src = tree_get_type(expr);

    switch (tree_get_kind(src))
    {
    case eTreeTypePointer:
        if (!util_types_equal(dst->ptr, src->ptr))
        {
            return tree_error(tree_get_node(expr),
                                "cannot cast unrelated pointer types `%s` to `%s`",
                                tree_to_string(src), tree_to_string(dst));
        }

        return cast_check_length(dst, expr, dst->length, src->length);

    default: return tree_error(tree_get_node(expr),
                                "cannot cast `%s` to `%s`",
                                tree_to_string(src), tree_to_string(dst));
    }
}

static tree_t *cast_to_digit(const tree_t *dst, tree_t *expr)
{
    CTASSERTF(tree_is(dst, eTreeTypeDigit), "(dst=%s)", tree_to_string(dst));

    const tree_t *src = tree_get_type(expr);

    switch (tree_get_kind(src))
    {
    case eTreeTypeDigit:
        if (dst->digit < src->digit)
        {
            return tree_error(tree_get_node(expr),
                                "cannot cast `%s` to `%s`, may truncate",
                                tree_to_string(src), tree_to_string(dst));
        }

        return expr;

    default:
        return tree_error(tree_get_node(expr),
                          "cannot cast `%s` to `%s`",
                          tree_to_string(src), tree_to_string(dst));
    }
}

static tree_t *cast_to_bool(const tree_t *dst, tree_t *expr)
{
    CTASSERTF(tree_is(dst, eTreeTypeBool), "(dst=%s)", tree_to_string(dst));

    const tree_t *src = tree_get_type(expr);

    switch (tree_get_kind(src))
    {
    case eTreeTypeBool:
        return expr;

    default:
        return tree_error(tree_get_node(expr),
                          "cannot cast `%s` to `%s`",
                          tree_to_string(src), tree_to_string(dst));
    }
}

tree_t *util_type_cast(const tree_t *dst, tree_t *expr)
{
    CTASSERTF(dst != NULL && expr != NULL, "(dst=%p, expr=%p)", dst, expr);

    const tree_t *src = tree_get_type(expr);

    if (util_types_equal(dst, src)) { return expr; }

    switch (tree_get_kind(dst))
    {
    case eTreeTypeOpaque:
        return cast_to_opaque(dst, expr);

    case eTreeTypePointer:
        return cast_to_pointer(dst, expr);

    case eTreeTypeReference:
        return util_type_cast(dst->ptr, expr);

    case eTreeTypeDigit:
        return cast_to_digit(dst, expr);

    case eTreeTypeBool:
        return cast_to_bool(dst, expr);

    default:
        return tree_error(tree_get_node(expr),
                          "cannot cast `%s` to `%s`",
                          tree_to_string(src), tree_to_string(dst));
    }
}

static bool eval_binary(mpz_t value, const tree_t *expr)
{
    CTASSERT(expr != NULL);

    mpz_t lhs;
    mpz_t rhs;
    mpz_init(lhs);
    mpz_init(rhs);

    if (!util_eval_digit(lhs, expr->lhs)) { return false; }
    if (!util_eval_digit(rhs, expr->rhs)) { return false; }

    switch (expr->binary)
    {
    case eBinaryAdd:
        mpz_add(value, lhs, rhs);
        break;
    case eBinarySub:
        mpz_sub(value, lhs, rhs);
        break;
    case eBinaryMul:
        mpz_mul(value, lhs, rhs);
        break;
    default:
        return false;
    }

    return true;
}

bool util_eval_digit(mpz_t value, const tree_t *expr)
{
    CTASSERT(expr != NULL);
    switch (tree_get_kind(expr))
    {
    case eTreeExprDigit:
        mpz_set(value, expr->digitValue);
        return true;

    case eTreeExprBinary:
        return eval_binary(value, expr);

    default:
        return false;
    }
}

tree_t *util_create_string(tree_t *sema, const node_t *node, tree_t *letter, const char *text, size_t length)
{
    // get basic info
    where_t where = get_node_location(node);

    // get current context
    tree_t *mod = util_current_module(sema);
    tree_t *symbol = util_current_symbol(sema);

    // create type and storage
    const tree_t *type = tree_type_pointer(node, "$", letter, length + 1);
    tree_storage_t storage = {
        .storage = letter,
        .size = length + 1,
        .quals = eQualConst
    };
    const char *id = format("%s$%" PRI_LINE "$%" PRI_COLUMN, tree_get_name(symbol), where.firstLine, where.firstColumn);
    tree_t *str = tree_expr_string(node, type, text, length);
    tree_t *decl = tree_decl_global(node, id, storage, type, str);

    tree_module_set(mod, eSemaValues, id, decl);

    return decl;
}

tree_t *util_create_call(tree_t *sema, const node_t *node, const tree_t *fn, vector_t *args)
{
    const tree_attribs_t *attribs = tree_get_attrib(fn);
    if (attribs->deprecated != NULL)
    {
        message_t *id = report(sema->reports, eWarn, node, "call to deprecated function `%s`", tree_get_name(fn));
        report_note(id, "deprecated: %s", attribs->deprecated);
    }

    return tree_expr_call(node, fn, args);
}

bool util_length_bounded(size_t length)
{
    return length != SIZE_MAX;
}

const char *util_length_name(size_t length)
{
    return util_length_bounded(length) ? format("%zu", length) : "unbounded";
}