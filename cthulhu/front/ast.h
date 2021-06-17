#pragma once

#include <stdbool.h>

#include "scanner.h"

typedef enum {
    /* literals */
    AST_DIGIT,
    AST_IDENT,
    AST_BOOL,

    /* operations */
    AST_UNARY,
    AST_BINARY,
    AST_TERNARY,
    AST_CALL,

    /* statements */
    AST_RETURN,
    AST_STMTS,

    /* types */
    AST_TYPENAME,

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
        /* AST_IDENT, AST_TYPENAME */
        char *text;

        /* AST_DIGIT */
        uint64_t digit;

        /* AST_BOOL */
        bool b;

        /* AST_FUNC */
        struct {
            char *name; /* name of function */
            node_t *result; /* return type */
            node_t *body; /* body */
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

        /* AST_STMTS */
        nodes_t *stmts;
    };
} node_t;

nodes_t *ast_list(node_t *init);
nodes_t *ast_empty(void);
nodes_t *ast_append(nodes_t *list, node_t *item);

node_t *ast_digit(scanner_t *x, YYLTYPE loc, char *text, int base);
node_t *ast_ident(scanner_t *x, YYLTYPE loc, char *text);
node_t *ast_bool(scanner_t *x, YYLTYPE loc, bool b);
node_t *ast_binary(scanner_t *x, YYLTYPE loc, node_t *lhs, node_t *rhs, int op);
node_t *ast_unary(scanner_t *x, YYLTYPE loc, node_t *expr, int op);
node_t *ast_ternary(scanner_t *x, YYLTYPE loc, node_t *cond, node_t *lhs, node_t *rhs);
node_t *ast_call(scanner_t *x, YYLTYPE loc, node_t *func);

node_t *ast_typename(scanner_t *x, YYLTYPE loc, char *name);

node_t *ast_return(scanner_t *x, YYLTYPE loc, node_t *expr);
node_t *ast_stmts(scanner_t *x, YYLTYPE loc, nodes_t *stmts);

node_t *ast_func(scanner_t *x, YYLTYPE loc, char *name, node_t *result, node_t *body);
