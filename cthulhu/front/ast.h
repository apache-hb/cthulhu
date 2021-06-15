#pragma once

#include "scanner.h"

typedef enum {
    /* literals */
    AST_DIGIT,
    AST_IDENT,

    /* operations */
    AST_UNARY,
    AST_BINARY,
    AST_TERNARY,
    AST_CALL,

    /* statements */
    AST_RETURN,

    /* decls */
    AST_FUNC
} node_type_t;

typedef struct node_t node_t;

typedef struct {
    node_t *data;
    size_t size;
    size_t len;
} nodes_t;

typedef struct node_t {
    node_type_t type;

    scanner_t *source;
    YYLTYPE loc;

    union {
        /* AST_IDENT */
        char *text;

        /* AST_DIGIT */
        uint64_t digit;

        /* AST_FUNC */
        struct {
            char *name;
            node_t *body;
        } func;

        /* AST_BINARY */
        struct {
            int op;
            node_t *lhs, *rhs;
        } binary;

        /* AST_UNARY */
        struct {
            int op;
            node_t *expr;
        } unary;

        /* AST_TERNARY */
        struct {
            node_t *cond, *lhs, *rhs;
        } ternary;

        /* AST_RETURN, AST_CALL */
        node_t *expr;
    };
} node_t;

nodes_t *ast_list(node_t *init);
nodes_t *ast_append(nodes_t *list, node_t *item);

node_t *ast_digit(char *text);
node_t *ast_ident(char *text);
node_t *ast_binary(node_t *lhs, node_t *rhs, int op);
node_t *ast_unary(node_t *expr, int op);
node_t *ast_ternary(node_t *cond, node_t *lhs, node_t *rhs);
node_t *ast_call(node_t *func);
node_t *ast_return(node_t *expr);
node_t *ast_func(char *name, node_t *body);
