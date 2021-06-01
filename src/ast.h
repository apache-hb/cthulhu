#pragma once

typedef enum {
    NODE_DIGIT,

    NODE_BINARY,
    NODE_UNARY,
    NODE_TERNARY
} node_type_t;

typedef struct node_t node_t;

typedef struct node_t {
    node_type_t type;

    union {
        char *text;

        struct binary_t {
            int op;
            node_t *lhs, *rhs;
        } binary;

        struct unary_t {
            int op;
            node_t *expr;
        } unary;

        struct ternary_t {
            node_t *cond, *lhs, *rhs;
        } ternary;
    };
} node_t;

node_t *ast_digit(char *text);
node_t *ast_binary(node_t *lhs, node_t *rhs, int op);
node_t *ast_unary(node_t *expr, int op);
node_t *ast_ternary(node_t *cond, node_t *lhs, node_t *rhs);
