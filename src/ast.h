#ifndef AST_H
#define AST_H

#include <stdint.h>

typedef struct node_t node_t;

typedef enum {
    NODE_DIGIT,
    NODE_UNARY,
    NODE_BINARY,
    NODE_TERNARY
} node_kind_t;

typedef enum {
    UNARY_ABS,
    UNARY_NEG
} unary_op_t;

typedef enum {
    BINARY_ADD,
    BINARY_SUB,
    BINARY_DIV,
    BINARY_MUL,
    BINARY_REM
} binary_op_t;

typedef struct node_t {
    node_kind_t kind;

    union {
        char* digit;

        struct {
            unary_op_t op;
            node_t *expr;
        } unary;

        struct {
            binary_op_t op;
            node_t *lhs;
            node_t *rhs;
        } binary;

        struct {
            node_t *cond;
            node_t *yes;
            node_t *no;
        } ternary;
    };
} node_t;


node_t*
new_digit(char* digit);

node_t*
new_unary(unary_op_t op, node_t *expr);

node_t*
new_binary(binary_op_t op, node_t *lhs, node_t *rhs);

node_t*
new_ternary(node_t *cond, node_t *yes, node_t *no);

void
dump_node(node_t *node);

#endif /* AST_H */
