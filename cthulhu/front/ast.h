#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum {
    AST_DIGIT,

    AST_UNARY,
    AST_BINARY,
    AST_TERNARY,

    AST_RETURN
} node_type_t;

typedef struct node_t node_t;

typedef struct {
    node_t *data;
    size_t size;
    size_t len;
} nodes_t;

typedef struct node_t {
    node_type_t type;

    union {
        /* AST_DIGIT */
        uint64_t digit;

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

        /* AST_RETURN */
        node_t *expr;
    };
} node_t;

nodes_t *ast_list(node_t *init);
nodes_t *ast_append(nodes_t *list, node_t *item);

node_t *ast_digit(char *text);
node_t *ast_binary(node_t *lhs, node_t *rhs, int op);
node_t *ast_unary(node_t *expr, int op);
node_t *ast_ternary(node_t *cond, node_t *lhs, node_t *rhs);
node_t *ast_return(node_t *expr);
