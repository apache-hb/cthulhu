#include "cthulhu/ast/ast.h"

#include "cthulhu/util/util.h"

static const node_t kBuiltinNode = {.scan = NULL, .where = {0, 0, 0, 0}};

node_t *node_new(scan_t *scan, where_t where)
{
    node_t *node = ctu_malloc(sizeof(node_t));
    node->scan = scan;
    node->where = where;
    return node;
}

const scan_t *get_node_scanner(const node_t *node)
{
    CTASSERT(node != node_invalid(), "get-scanner on invalid node");

    return node->scan;
}

where_t get_node_location(const node_t *node)
{
    CTASSERT(node != node_invalid(), "get-location on invalid node");

    return node->where;
}

const node_t *node_builtin(void)
{
    return &kBuiltinNode;
}

const node_t *node_invalid(void)
{
    return NULL;
}
