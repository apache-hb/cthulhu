#include "ast.h"

#include "bison.h"

#include "cthulhu/util/report.h"
#include "cthulhu/util/util.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static node_t *ast(scanner_t *x, YYLTYPE loc, node_type_t type) {
    node_t *node = malloc(sizeof(node_t));
    node->type = type;
    node->source = x;
    node->loc = loc;
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
    ENSURE_SIZE(list->data, list->len, list->size, sizeof(node_t), 4);
    memcpy(list->data + list->len, item, sizeof(node_t));
    list->len += 1;
    return list;
}

nodes_t *ast_list(node_t *init) {
    nodes_t *it = nodes(4);
    return ast_append(it, init);
}

node_t *ast_digit(scanner_t *x, YYLTYPE loc, char *text, int base) {
    node_t *node = ast(x, loc, AST_DIGIT);
    uint64_t num = strtoull(text, NULL, base);

    if (num == UINT64_MAX) {
        reportf("integer overflow `%s` > UINT64_MAX", text);
    }

    node->digit = num;
    return node;
}

node_t *ast_ident(scanner_t *x, YYLTYPE loc, char *text) {
    node_t *node = ast(x, loc, AST_IDENT);
    node->text = text;
    return node;
}

node_t *ast_bool(scanner_t *x, YYLTYPE loc, bool b) {
    node_t *node = ast(x, loc, AST_BOOL);
    node->b = b;
    return node;
}

node_t *ast_binary(scanner_t *x, YYLTYPE loc, node_t *lhs, node_t *rhs, int op) {
    node_t *node = ast(x, loc, AST_BINARY);
    node->binary.op = op;
    node->binary.lhs = lhs;
    node->binary.rhs = rhs;
    return node;
}

node_t *ast_unary(scanner_t *x, YYLTYPE loc, node_t *expr, int op) {
    node_t *node = ast(x, loc, AST_UNARY);
    node->unary.op = op;
    node->unary.expr = expr;
    return node;
}

node_t *ast_ternary(scanner_t *x, YYLTYPE loc, node_t *cond, node_t *lhs, node_t *rhs) {
    node_t *node = ast(x, loc, AST_TERNARY);
    node->ternary.cond = cond;
    node->ternary.lhs = lhs;
    node->ternary.rhs = rhs;
    return node;
}

node_t *ast_call(scanner_t *x, YYLTYPE loc, node_t *func) {
    node_t *node = ast(x, loc, AST_CALL);
    node->expr = func;
    return node;
}

node_t *ast_return(scanner_t *x, YYLTYPE loc, node_t *expr) {
    node_t *node = ast(x, loc, AST_RETURN);
    node->expr = expr;
    return node;
}

node_t *ast_func(scanner_t *x, YYLTYPE loc, char *name, node_t *body) {
    node_t *node = ast(x, loc, AST_FUNC);
    node->func.name = name;
    node->func.body = body;
    return node;
}

node_t *ast_stmts(scanner_t *x, YYLTYPE loc, nodes_t *stmts) {
    node_t *node = ast(x, loc, AST_STMTS);
    node->stmts = stmts;
    return node;
}
