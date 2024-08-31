// SPDX-License-Identifier: GPL-3.0-only

#include "ctu/sema/default.h"

#include "base/panic.h"
#include "cthulhu/tree/query.h"
#include "cthulhu/tree/tree.h"
#include "ctu/sema/sema.h"

static tree_t *get_zero(const node_t *node, const tree_t *type)
{
    tree_t *value = tree_expr_digit_int(node, type, 0);
    return tree_expr_cast(node, type, value, eCastBit);
}

static tree_t *get_null(const node_t *node, const tree_t *type)
{
    tree_t *digit = ctu_get_int_type(eDigitPtr, eSignUnsigned);
    tree_t *value = get_zero(node, digit);
    return tree_expr_cast(node, type, value, eCastBit);
}

const tree_t *ctu_get_default_value(const node_t *node, const tree_t *type)
{
    type = tree_follow_type(type);
    tree_kind_t kind = tree_get_kind(type);
    switch (kind)
    {
    case eTreeTypeOpaque:
    case eTreeTypePointer:
        return get_null(node, type);
    case eTreeTypeDigit:
        return get_zero(node, type);
    case eTreeTypeBool:
        return tree_expr_bool(node, type, false);

    case eTreeTypeEmpty:
    case eTreeTypeUnit:
        CT_NEVER("cannot get default value for empty or unit type");

    default:
        CT_NEVER("unimplemented default value for type %s", tree_to_string(type));
    }
}
