#include "cthulhu/util/util.h"

#include "cthulhu/tree/query.h"

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

    case eTreeTypePointer:
        return util_types_equal(lhs->pointer, rhs->pointer);

    case eTreeTypeArray:
        return util_types_equal(lhs->array, rhs->array) && (lhs->length == rhs->length);

    default:
        return false;
    }
}

tree_t *util_type_cast(const tree_t *dst, tree_t *expr)
{
    CTASSERTF(dst != NULL && expr != NULL, "(dst=%p, expr=%p)", dst, expr);

    const tree_t *src = tree_get_type(expr);

    if (util_types_equal(dst, src)) { return expr; }

    tree_kind_t dstKind = tree_get_kind(dst);
    tree_kind_t srcKind = tree_get_kind(src);

    switch (dstKind)
    {
    case eTreeTypeArray:
        if (srcKind != eTreeTypeArray)
        {
            return tree_error(tree_get_node(expr),
                              "cannot cast `%s` to `%s`",
                              tree_to_string(src), tree_to_string(dst));
        }

        if (!util_types_equal(dst->array, src->array))
        {
            return tree_error(tree_get_node(expr),
                              "cannot unrelated array types `%s` to `%s`",
                              tree_to_string(src), tree_to_string(dst));
        }

        if (dst->length < src->length)
        {
            return tree_error(tree_get_node(expr),
                              "cannot truncate array length from %s to %s",
                              util_length_name(src->length),
                              util_length_name(dst->length));
        }

        return tree_expr_cast(tree_get_node(expr), dst, expr);

    default:
        return tree_error(tree_get_node(expr),
                          "cannot cast `%s` to `%s`",
                          tree_to_string(src), tree_to_string(dst));
    }
}

bool util_length_bounded(size_t length)
{
    return length != SIZE_MAX;
}

const char *util_length_name(size_t length)
{
    return util_length_bounded(length) ? format("%zu", length) : "unbounded";
}