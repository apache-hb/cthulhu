// SPDX-License-Identifier: GPL-3.0-only

#include "ctu/sema/decl/resolve.h"

#include "core/macros.h"
#include "cthulhu/tree/query.h"
#include "cthulhu/util/types.h"
#include "cthulhu/util/util.h"

#include "base/panic.h"

ctu_t *begin_resolve(tree_t *sema, tree_t *self, void *user, ctu_kind_t kind)
{
    ctu_t *decl = user;
    CTASSERTF(decl->kind == kind, "decl %s is not a %d", decl->name, kind);

    CT_UNUSED(sema);
    CT_UNUSED(self);

    return decl;
}

static bool is_array(const tree_t *type)
{
    return tree_is(type, eTreeTypeArray);
}

tree_t *ctu_cast_type(tree_t *sema, tree_t *expr, const tree_t *type)
{
    CTASSERT(sema != NULL);
    CTASSERT(expr != NULL);
    CTASSERT(type != NULL);

    const tree_t *inner = tree_get_type(expr);

    // TODO: deduplicate casting logic

    // if we're casting to a pointer we should preserve the length information
    if (is_array(inner) && tree_is(type, eTreeTypePointer))
    {
        if (util_types_equal(inner->ptr, type->ptr))
        {
            const tree_t *elem = tree_ty_load_type(inner);
            tree_t *ptr = tree_type_pointer(tree_get_node(expr), tree_get_name(type), elem, inner->length);
            return tree_expr_cast(expr->node, ptr, expr);
        }
    }

    if (is_array(type) && tree_is(inner, eTreeTypePointer))
    {
        if (util_types_equal(inner->ptr, type->ptr))
        {
            return tree_expr_cast(expr->node, type, expr);
        }
    }

    if (is_array(type) && is_array(inner))
    {
        if (type->length >= inner->length)
        {
            if (util_types_equal(type->ptr, inner->ptr))
            {
                return tree_expr_cast(expr->node, type, expr);
            }
        }
    }

    // TODO: deal with other casts
    return expr;
}
