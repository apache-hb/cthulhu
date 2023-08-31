#include "cthulhu/util/util.h"

#include "cthulhu/tree/query.h"

#include "base/panic.h"

#include <stdlib.h>

void *util_select_decl(tree_t *sema, const size_t *tags, size_t len, const char *name)
{
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
    case eTreeTypeString:
        return true;

    case eTreeTypeDigit:
        return (lhs->digit == rhs->digit) && (lhs->sign == rhs->sign);

    case eTreeTypePointer:
        return util_types_equal(lhs->pointer, rhs->pointer);

    default:
        return false;
    }
}

bool util_length_bounded(size_t length)
{
    return length != SIZE_MAX;
}
