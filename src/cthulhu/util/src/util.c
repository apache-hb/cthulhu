// SPDX-License-Identifier: LGPL-3.0-only

#include "cthulhu/util/util.h"
#include "cthulhu/events/events.h"
#include "cthulhu/util/types.h"

#include "cthulhu/tree/query.h"

#include "memory/memory.h"
#include "std/str.h"

#include "base/panic.h"

#include <stdint.h>

void *util_select_decl(tree_t *sema, const size_t *tags, size_t len, const char *name)
{
    CTASSERT(tags != NULL);
    CTASSERT(len > 0);

    for (size_t i = 0; i < len; i++)
    {
        tree_t *decl = tree_module_get(sema, tags[i], name);
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
    case eTreeTypeBool: return true;

    case eTreeTypeDigit: return (lhs->digit == rhs->digit) && (lhs->sign == rhs->sign);

    case eTreeTypeReference:
    case eTreeTypePointer: return util_types_equal(lhs->ptr, rhs->ptr);

    default: return false;
    }
}

bool util_types_comparable(const tree_t *lhs, const tree_t *rhs)
{
    if (util_types_equal(lhs, rhs))
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
    case eTreeTypeBool:
    case eTreeTypeDigit: return true;

    default: return false; /* TODO probably wrong */
    }
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

    switch (tree_get_kind(src))
    {
    case eTreeTypeDigit:
        if (dst->digit < src->digit)
        {
            return tree_error(tree_get_node(expr), &kEvent_InvalidCast,
                              "cannot cast `%s` to `%s`, may truncate", tree_to_string(src),
                              tree_to_string(dst));
        }

        return expr;

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

bool util_eval_digit(mpz_t value, const tree_t *expr)
{
    CTASSERT(expr != NULL);
    switch (tree_get_kind(expr))
    {
    case eTreeExprDigit: mpz_set(value, expr->digit_value); return true;

    case eTreeExprBinary: return eval_binary(value, expr);

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
