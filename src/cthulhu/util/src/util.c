// SPDX-License-Identifier: LGPL-3.0-only

#include "cthulhu/util/util.h"
#include "cthulhu/events/events.h"
#include "cthulhu/util/types.h"

#include "cthulhu/tree/query.h"

#include "memory/memory.h"
#include "std/str.h"

#include "base/panic.h"

#include <stdint.h>

void *util_select_decl(tree_t *sema, search_t search, const char *name)
{
    CTASSERT(search.tags != NULL);
    CTASSERT(search.count > 0);

    for (size_t i = 0; i < search.count; i++)
    {
        tree_t *decl = tree_module_get(sema, search.tags[i], name);
        if (decl != NULL)
        {
            return decl;
        }
    }

    return NULL;
}

bool util_types_equal(const tree_t *lhs, const tree_t *rhs)
{
    CTASSERTF(lhs != NULL && rhs != NULL, "(lhs=%p, rhs=%p)", (void *)lhs, (void *)rhs);

    lhs = tree_follow_type(lhs);
    rhs = tree_follow_type(rhs);

    if (lhs == rhs)
    {
        return true;
    }

    tree_kind_t lhs_kind = tree_get_kind(lhs);
    tree_kind_t rhs_kind = tree_get_kind(rhs);

    if (lhs_kind != rhs_kind)
    {
        return false;
    }

    switch (lhs_kind)
    {
    case eTreeTypeEmpty:
    case eTreeTypeUnit:
    case eTreeTypeOpaque:
    case eTreeTypeBool: return true;

    case eTreeTypeDigit: return (lhs->digit == rhs->digit) && (lhs->sign == rhs->sign);

    case eTreeTypeReference:
    case eTreeTypePointer: return util_types_equal(lhs->ptr, rhs->ptr);

    default: return false;
    }
}

bool util_types_comparable(tree_cookie_t *cookie, const tree_t *lhs, const tree_t *rhs)
{
    lhs = tree_follow_type(tree_resolve_type(cookie, lhs));
    rhs = tree_follow_type(tree_resolve_type(cookie, rhs));

    if (util_types_equal(lhs, rhs))
    {
        return true;
    }

    tree_kind_t lhs_kind = tree_get_kind(lhs);
    tree_kind_t rhs_kind = tree_get_kind(rhs);

    if (lhs_kind == rhs_kind)
    {
        switch (lhs_kind)
        {
        case eTreeTypeBool:
        case eTreeTypeOpaque:
        case eTreeTypeDigit:
            return true;

        default: break;
        }
    }

    if (util_type_is_pointer(lhs) && util_type_is_opaque(rhs))
        return true;

    if (util_type_is_opaque(lhs) && util_type_is_pointer(rhs))
        return true;

    return false;
}

static bool can_cast_length(size_t dst, size_t src)
{
    if (!util_length_bounded(dst))
    {
        return true;
    }
    if (!util_length_bounded(src))
    {
        return true;
    }

    return dst < src;
}

static tree_t *cast_check_length(const tree_t *dst, tree_t *expr, size_t dstlen, size_t srclen)
{
    if (!can_cast_length(dstlen, srclen))
    {
        return tree_error(
            tree_get_node(expr), &kEvent_InvalidCast,
            "creating an array with a length greater than its backing memory is unwise. (%s < %s)",
            util_length_name(srclen), util_length_name(dstlen));
    }

    return tree_expr_cast(tree_get_node(expr), dst, expr, eCastBit);
}

static tree_t *cast_to_opaque(const tree_t *dst, tree_t *expr)
{
    CTASSERTF(tree_is(dst, eTreeTypeOpaque), "(dst=%s)", tree_to_string(dst));

    const tree_t *src = tree_get_type(expr);

    switch (tree_get_kind(src))
    {
    case eTreeTypePointer:
    case eTreeTypeDigit:
        return tree_expr_cast(tree_get_node(expr), dst, expr, eCastBit); // TODO: a little iffy

    default:
        return tree_error(tree_get_node(expr), &kEvent_InvalidCast, "cannot cast `%s` to `%s`",
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
            return tree_error(tree_get_node(expr), &kEvent_InvalidCast,
                              "cannot cast unrelated pointer types `%s` to `%s`",
                              tree_to_string(src), tree_to_string(dst));
        }

        return cast_check_length(dst, expr, dst->length, src->length);

    default:
        return tree_error(tree_get_node(expr), &kEvent_InvalidCast, "cannot cast `%s` to `%s`",
                          tree_to_string(src), tree_to_string(dst));
    }
}

static tree_t *cast_to_digit(const tree_t *dst, tree_t *expr)
{
    CTASSERTF(tree_is(dst, eTreeTypeDigit), "(dst=%s)", tree_to_string(dst));

    const tree_t *src = tree_get_type(expr);

    if (util_type_is_pointer(src) && dst->digit == eDigitPtr)
    {
        return tree_expr_cast(tree_get_node(expr), dst, expr, eCastBit);
    }

    // TODO: need to distinguish between explicit and implicit casts
    switch (tree_get_kind(src))
    {
    case eTreeTypeDigit:
        return tree_expr_cast(tree_get_node(expr), dst, expr, eCastSignExtend);

    default:
        return tree_error(tree_get_node(expr), &kEvent_InvalidCast, "cannot cast `%s` to `%s`",
                          tree_to_string(src), tree_to_string(dst));
    }
}

static tree_t *cast_to_bool(const tree_t *dst, tree_t *expr)
{
    CTASSERTF(tree_is(dst, eTreeTypeBool), "(dst=%s)", tree_to_string(dst));

    const tree_t *src = tree_get_type(expr);

    switch (tree_get_kind(src))
    {
    case eTreeTypeBool: return expr;

    default:
        return tree_error(tree_get_node(expr), &kEvent_InvalidCast, "cannot cast `%s` to `%s`",
                          tree_to_string(src), tree_to_string(dst));
    }
}

tree_t *util_type_cast(const tree_t *dst, tree_t *expr)
{
    CTASSERTF(dst != NULL && expr != NULL, "(dst=%p, expr=%p)", (void *)dst, (void *)expr);

    const tree_t *src = tree_get_type(expr);

    dst = tree_follow_type(dst);

    if (util_types_equal(dst, src))
    {
        return expr;
    }

    switch (tree_get_kind(dst))
    {
    case eTreeTypeOpaque: return cast_to_opaque(dst, expr);

    case eTreeTypePointer: return cast_to_pointer(dst, expr);

    case eTreeTypeReference: return util_type_cast(dst->ptr, expr);

    case eTreeTypeDigit: return cast_to_digit(dst, expr);

    case eTreeTypeBool: return cast_to_bool(dst, expr);

    default:
        return tree_error(tree_get_node(expr), &kEvent_InvalidCast, "cannot cast `%s` to `%s`",
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

    if (!util_eval_digit(lhs, expr->lhs))
    {
        return false;
    }
    if (!util_eval_digit(rhs, expr->rhs))
    {
        return false;
    }

    switch (expr->binary)
    {
    case eBinaryAdd: mpz_add(value, lhs, rhs); break;
    case eBinarySub: mpz_sub(value, lhs, rhs); break;
    case eBinaryMul: mpz_mul(value, lhs, rhs); break;
    default: return false;
    }

    return true;
}

static bool eval_cast(mpz_t value, const tree_t *expr)
{
    CTASSERT(expr != NULL);

    mpz_t src;
    mpz_init(src);

    if (!util_eval_digit(src, expr->expr))
    {
        return false;
    }

    switch (expr->cast)
    {
    case eCastSignExtend: mpz_set(value, src); break;
    default: return false;
    }

    return true;
}

bool util_eval_digit(mpz_t value, const tree_t *expr)
{
    CTASSERT(expr != NULL);
    switch (tree_get_kind(expr))
    {
    case eTreeExprDigit:
        mpz_set(value, expr->digit_value);
        return true;

    case eTreeExprBinary:
        return eval_binary(value, expr);

    case eTreeExprCast:
        return eval_cast(value, expr);

    default: return false;
    }
}

bool util_length_bounded(size_t length)
{
    return length != SIZE_MAX;
}

const char *util_length_name(size_t length)
{
    arena_t *arena = get_global_arena();
    return util_length_bounded(length) ? str_format(arena, "%zu", length) : "unbounded";
}

bool util_type_is_aggregate(const tree_t *type)
{
    CTASSERTF(!tree_is(type, eTreeTypeAlias), "(type=%s)", tree_to_string(type));
    return tree_is(type, eTreeTypeUnion) || tree_is(type, eTreeTypeStruct);
}

bool util_type_is_pointer(const tree_t *type)
{
    CTASSERTF(!tree_is(type, eTreeTypeAlias), "(type=%s)", tree_to_string(type));
    return tree_is(type, eTreeTypePointer);
}

bool util_type_is_array(const tree_t *type)
{
    CTASSERTF(!tree_is(type, eTreeTypeAlias), "(type=%s)", tree_to_string(type));
    return tree_is(type, eTreeTypeArray);
}

bool util_type_is_opaque(const tree_t *type)
{
    CTASSERTF(!tree_is(type, eTreeTypeAlias), "(type=%s)", tree_to_string(type));
    return tree_is(type, eTreeTypeOpaque);
}

bool util_type_is_reference(const tree_t *type)
{
    CTASSERTF(!tree_is(type, eTreeTypeAlias), "(type=%s)", tree_to_string(type));
    return tree_is(type, eTreeTypeReference);
}

bool util_type_is_digit(const tree_t *type)
{
    CTASSERTF(!tree_is(type, eTreeTypeAlias), "(type=%s)", tree_to_string(type));
    return tree_is(type, eTreeTypeDigit);
}
