#include "ast.h"

#include <string.h>
#include <stdlib.h>

static node_t *ast(node_type_t type) {
    node_t *node = malloc(sizeof(node_t));
    node->type = type;
    return node;
}

static nodes_t *nodes(size_t init) {
    nodes_t *it = malloc(sizeof(nodes_t));
    it->data = malloc(sizeof(node_t) * init);
    it->len = 0;
    it->size = init;
    return it;
}

nodes_t *ast_empty(void) {
    return nodes(4);
}

nodes_t *ast_append(nodes_t *list, node_t *item) {
    if (list->len + 1 < list->size) {
        list->size += 4;
        list->data = realloc(list->data, sizeof(node_t) * list->size);
    }
    memcpy(list->data + list->len, item, sizeof(node_t));
    list->len += 1;
    return list;
}

nodes_t *ast_list(node_t *init) {
    nodes_t *it = nodes(4);
    return ast_append(it, init);
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

node_t *ast_call(node_t *expr, nodes_t *args) {
    node_t *node = ast(NODE_CALL);
    node->call.expr = expr;
    node->call.args = args;
    return node;
}
