#include "ast.h"

#include "ctu/util/util.h"

node_t *node_new(scan_t *scan, where_t where) {
    node_t *node = arena_malloc(&scan->nodes, sizeof(node_t));
    node->scan = scan;
    node->where = where;
    return node;
}
