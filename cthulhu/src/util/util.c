#include "cthulhu/util/util.h"

#include "cthulhu/tree/query.h"
#include "cthulhu/tree/sema.h"

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

    case eTreeTypeStorage:
    case eTreeTypePointer:
        return util_types_equal(lhs->ptr, rhs->ptr);

    default:
        return false;
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

static tree_t *cast_to_pointer(const tree_t *dst, tree_t *expr)
{
    CTASSERTF(tree_is(dst, eTreeTypePointer), "(dst=%s)", tree_to_string(dst));

    const tree_t *src = tree_get_type(expr);

    switch (tree_get_kind(src))
    {
    case eTreeTypeStorage:
        if (!util_types_equal(dst->ptr, src->ptr))
        {
            return tree_error(tree_get_node(expr),
                                "cannot cast unrelated array types `%s` to `%s`",
                                tree_to_string(src), tree_to_string(dst));
        }

        return cast_check_length(dst, expr, dst->length, src->length);

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

tree_t *util_type_cast(const tree_t *dst, tree_t *expr)
{
    CTASSERTF(dst != NULL && expr != NULL, "(dst=%p, expr=%p)", dst, expr);

    const tree_t *src = tree_get_type(expr);

    if (util_types_equal(dst, src)) { return expr; }

    switch (tree_get_kind(dst))
    {
    case eTreeTypePointer:
        return cast_to_pointer(dst, expr);

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