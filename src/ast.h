#pragma once

typedef enum {
    NODE_DIGIT,

    NODE_BINARY
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
    };
} node_t;

node_t *ast_digit(char *text);
node_t *ast_binary(node_t *lhs, node_t *rhs, int op);
