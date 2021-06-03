#include "ast.h"

#include <stdlib.h>

static node_t *ast(node_type_t type) {
    node_t *node = malloc(sizeof(node_t));
    node->type = type;
    return node;
}

node_t *ast_digit(char *text) {
    node_t *node = ast(NODE_DIGIT);
    node->text = text;
    return node;
}

node_t *ast_binary(node_t *lhs, node_t *rhs, int op) {
    node_t *node = ast(NODE_BINARY);
    node->binary.op = op;
    node->binary.lhs = lhs;
    node->binary.rhs = rhs;
    return node;
}

node_t *ast_unary(node_t *expr, int op) {
    node_t *node = ast(NODE_UNARY);
    node->unary.op = op;
    node->unary.expr = expr;
    return node;
}

node_t *ast_ternary(node_t *cond, node_t *lhs, node_t *rhs) {
    node_t *node = ast(NODE_TERNARY);
    node->ternary.cond = cond;
    node->ternary.lhs = lhs;
    node->ternary.rhs = rhs;
    return node;
}

node_t *ast_return(node_t *expr) {
    node_t *node = ast(NODE_RETURN);
    node->expr = expr;
    return node;
}