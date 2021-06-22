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
    node->typeof = NULL;

    return node;
}

const char *node_name(node_t *node) {
    switch (node->kind) {
    case AST_TYPE: return node->nameof;
    case AST_DECL_FUNC: case AST_DECL_VAR: case AST_DECL_PARAM:
        return node->name;

    default:
        return NULL;
    }
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

node_t *ast_symbol(scanner_t *scanner, where_t where, char *text) {
    node_t *node = new_node(scanner, where, AST_SYMBOL);

    node->ident = text;

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

node_t *ast_call(scanner_t *scanner, where_t where, node_t *body, nodes_t *args) {
    node_t *node = new_node(scanner, where, AST_CALL);

    node->expr = body;
    node->args = args;

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

node_t *ast_branch(scanner_t *scanner, where_t where, node_t *cond, node_t *branch) {
    node_t *node = new_node(scanner, where, AST_BRANCH);

    node->cond = cond;
    node->branch = branch;

    return node;
}

node_t *ast_decl_func(
    scanner_t *scanner, where_t where, 
    char *name, nodes_t *params,
    node_t *result, node_t *body) {

    node_t *node = new_node(scanner, where, AST_DECL_FUNC);

    node->name = name;
    node->params = params;
    node->result = result;
    node->body = body;

    return node;
}

node_t *ast_decl_param(scanner_t *scanner, where_t where, char *name, node_t *type) {
    node_t *node = new_node(scanner, where, AST_DECL_PARAM);

    node->name = name;
    node->type = type;

    return node;
}

static const where_t NOWHERE = { 0, 0, 0, 0 };

node_t *ast_type(const char *name, type_t *typeof) {
    node_t *node = new_node(NULL, NOWHERE, AST_TYPE);
    node->nameof = name;

    connect_type(node, typeof);

    return node;
}

void connect_type(node_t *node, type_t *type) {
    /**
     * tell the type that this is the parent node
     * for easier error reporting and decl lookup.
     * circular reference, take that rust :P
     */
    node->typeof = type;
    type->node = node;
}
