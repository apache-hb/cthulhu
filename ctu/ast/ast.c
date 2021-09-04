#include "ast.h"

#include "ctu/util/util.h"

node_t *node_new(scan_t *scan, where_t where) {
    node_t *node = ctu_malloc(sizeof(node_t));
    node->scan = scan;
    node->where = where;
    return node;
}

report_t reportn(level_t lvl, const node_t *node, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    report_t id = reportv(lvl, node->scan, node->where, fmt, args);
    va_end(args);
    return id;
}

void report_appendn(report_t id, const node_t *node, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    report_appendv(id, node->scan, node->where, fmt, args);
    va_end(args);
}
