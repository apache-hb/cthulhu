#include "cthulhu/ast/ast.h"

#include "cthulhu/util/util.h"

node_t *node_new(scan_t *scan, where_t where) {
    node_t *node = ctu_malloc(sizeof(node_t));
    node->scan = scan;
    node->where = where;
    return node;
}

node_t *node_builtin(scan_t *scan) {
    where_t where = { 0, 0, 0, 0 };
    return node_new(scan, where);
}

node_t *node_last_line(const node_t *node) {
    where_t where = node->where;

    where_t result = {
        .first_line = where.last_line,
        .first_column = 0,
        .last_column = where.last_column,
        .last_line = where.last_line
    };

    return node_new(node->scan, result);
}

node_t *node_merge(const node_t *lhs, const node_t *rhs) {    
    if (lhs->scan != rhs->scan) {
        return NULL;
    }

    where_t lloc = lhs->where;
    where_t rloc = rhs->where;

    where_t where = {
        .first_line = MIN(lloc.first_line, rloc.first_line),
        .first_column = MIN(lloc.first_column, rloc.first_column),
        .last_line = MAX(lloc.last_line, rloc.last_line),
        .last_column = MAX(lloc.last_column, rloc.last_column)
    };

    return node_new(lhs->scan, where);
}
