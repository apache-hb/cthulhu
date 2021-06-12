#include "ast.h"

#include "bison.h"

#include "cthulhu/report/report.h"
#include "cthulhu/util.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

nodes_t *ast_append(nodes_t *list, node_t *item) {
    ENSURE_SIZE(list->data, list->len, list->size, sizeof(node_t), 4);
    memcpy(list->data + list->len, item, sizeof(node_t));
    list->len += 1;
    return list;
}

nodes_t *ast_list(node_t *init) {
    nodes_t *it = nodes(4);
    return ast_append(it, init);
}

node_t *ast_digit(char *text) {
    node_t *node = ast(AST_DIGIT);
    uint64_t num = strtoull(text, NULL, 10);

    if (num == UINT64_MAX) {
        reportf("integer overflow `%s` > UINT64_MAX", text);
    }

    node->digit = num;
    return node;
}

node_t *ast_binary(node_t *lhs, node_t *rhs, int op) {
    node_t *node = ast(AST_BINARY);
    node->binary.op = op;
    node->binary.lhs = lhs;
    node->binary.rhs = rhs;
    return node;
}

node_t *ast_unary(node_t *expr, int op) {
    node_t *node = ast(AST_UNARY);
    node->unary.op = op;
    node->unary.expr = expr;
    return node;
}

node_t *ast_ternary(node_t *cond, node_t *lhs, node_t *rhs) {
    node_t *node = ast(AST_TERNARY);
    node->ternary.cond = cond;
    node->ternary.lhs = lhs;
    node->ternary.rhs = rhs;
    return node;
}

node_t *ast_return(node_t *expr) {
    node_t *node = ast(AST_RETURN);
    node->expr = expr;
    return node;
}