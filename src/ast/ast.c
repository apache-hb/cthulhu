#include "cthulhu/ast/ast.h"

#include "cthulhu/util/util.h"

node_t *node_new(scan_t *scan, where_t where)
{
    node_t *node = ctu_malloc(sizeof(node_t));
    node->scan = scan;
    node->where = where;
    return node;
}

static const node_t kBuiltinNode = {.scan = NULL, .where = {0, 0, 0, 0}};

const node_t *node_builtin(void)
{
    return &kBuiltinNode;
}
