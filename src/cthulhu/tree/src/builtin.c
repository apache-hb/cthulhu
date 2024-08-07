#include "cthulhu/tree/builtin.h"

#include "common.h"
#include "cthulhu/tree/ops.h"

tree_t *tree_builtin_sizeof(const node_t *node, const tree_t *type, const tree_t *size_type)
{
    tree_t *result = tree_new(eTreeExprSizeOf, node, size_type);
    result->object = type;
    return result;
}

tree_t *tree_builtin_alignof(const node_t *node, const tree_t *type, const tree_t *align_type)
{
    tree_t *result = tree_new(eTreeExprAlignOf, node, align_type);
    result->object = type;
    return result;
}

tree_t *tree_builtin_offsetof(const node_t *node, const tree_t *type, const tree_t *field, const tree_t *offset_type)
{
    tree_t *result = tree_new(eTreeExprOffsetOf, node, offset_type);
    result->object = type;
    result->field = field;
    return result;
}

