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
    node_t *node = NEW(node_t);
    node->scan = scan;
    node->where = where;
    return node;
}
