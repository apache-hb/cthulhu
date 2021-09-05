#include "ast.h"

#include "ctu/util/util.h"


const char *binary_name(binary_t op) {
    switch (op) {
    case BINARY_ADD: return "add";
    case BINARY_SUB: return "sub";
    case BINARY_MUL: return "mul";
    case BINARY_DIV: return "div";
    case BINARY_REM: return "rem";

    case BINARY_EQ: return "eq";
    case BINARY_NEQ: return "neq";
    case BINARY_LT: return "lt";
    case BINARY_LTE: return "lte";
    case BINARY_GT: return "gt";
    case BINARY_GTE: return "gte";

    default: return "???";
    }
}

const char *unary_name(unary_t op) {
    switch (op) {
    case UNARY_ABS: return "abs";
    case UNARY_NEG: return "neg";
    default: return "???";
    }
}

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
