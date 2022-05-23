#include "cthulhu/ast/ast.h"

#include "cthulhu/util/util.h"

typedef struct
{
    scan_t *scan;  ///< the source file
    where_t where; ///< the location of this node in the source file
} node_data_t;

static const node_data_t kBuiltinNode = {.scan = NULL, .where = {0, 0, 0, 0}};

node_t node_new(scan_t *scan, where_t where)
{
    node_data_t *node = ctu_malloc(sizeof(node_data_t));
    node->scan = scan;
    node->where = where;
    return node;
}

const scan_t *get_node_scanner(node_t node)
{
    const node_data_t *self = node;

    return self->scan;
}

where_t get_node_location(node_t node)
{
    const node_data_t *self = node;

    return self->where;
}

node_t node_builtin(void)
{
    return &kBuiltinNode;
}

node_t node_invalid(void)
{
    return NULL;
}

bool node_is_valid(node_t node)
{
    return node != node_invalid();
}
