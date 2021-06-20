#include "ast.h"

#include <stdlib.h>
#include <string.h>

#include "ctu/util/str.h"

#include "ctu/debug/ast.h"

static node_t *new_node(scanner_t *scanner, where_t where, ast_t kind) {
    node_t *node = malloc(sizeof(node_t));

    node->kind = kind;
    node->scanner = scanner;
    node->where = where;

    return node;
}

nodes_t *ast_append(nodes_t *list, node_t *node) {
    if (list->len + 1 >= list->size) {
        list->size += 4;
        list->data = realloc(list->data, sizeof(node_t) * list->size);
    }
    memcpy(list->data + list->len, node, sizeof(node_t));
    list->len += 1;

    return list;
}

nodes_t *ast_list(node_t *init) {
    nodes_t *nodes = malloc(sizeof(nodes_t));

    nodes->data = malloc(sizeof(node_t) * 4);
    nodes->len = 0;
    nodes->size = 4;

    if (init)
        ast_append(nodes, init);

    return nodes;
}

node_t *ast_digit(scanner_t *scanner, where_t where, char *digit) {
    node_t *node = new_node(scanner, where, AST_DIGIT);

    uint64_t out = strtoull(digit, NULL, 10);

    node->digit = out;

    return node;
}

node_t *ast_ident(scanner_t *scanner, where_t where, char *text) {
    node_t *node = new_node(scanner, where, AST_IDENT);

    node->ident = strdup(text);

    return node;
}

node_t *ast_unary(scanner_t *scanner, where_t where, unary_t unary, node_t *expr) {
    node_t *node = new_node(scanner, where, AST_UNARY);

    node->unary = unary;
    node->expr = expr;

    return node;
}

node_t *ast_binary(scanner_t *scanner, where_t where, binary_t binary, node_t *lhs, node_t *rhs) {
    node_t *node = new_node(scanner, where, AST_BINARY);

    node->binary = binary;
    node->lhs = lhs;
    node->rhs = rhs;

    return node;
}

node_t *ast_stmts(scanner_t *scanner, where_t where, nodes_t *stmts) {
    node_t *node = new_node(scanner, where, AST_STMTS);

    node->stmts = stmts;

    return node;
}

node_t *ast_return(scanner_t *scanner, where_t where, node_t *expr) {
    node_t *node = new_node(scanner, where, AST_RETURN);

    node->expr = expr;

    return node;
}