#ifndef AST_H
#define AST_H

#include <stdint.h>
#include <stddef.h>

typedef struct node_t node_t;
typedef struct nodes_t nodes_t;

typedef enum {
    NODE_DIGIT,
    NODE_UNARY,
    NODE_BINARY,
    NODE_TERNARY,
    NODE_CALL,
    NODE_FUNC,
    NODE_COMPOUND,
    NODE_NAME,
    NODE_RETURN,
    NODE_TYPENAME,
    NODE_POINTER,
    NODE_PARAM
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

typedef struct nodes_t {
    size_t length;
    size_t size;
    struct node_t *data;
} nodes_t;

typedef struct node_t {
    node_kind_t kind;

    union {
        char *digit;
        char *name;

        struct unary_t {
            unary_op_t op;
            node_t *expr;
        } unary;

        struct binary_t {
            binary_op_t op;
            node_t *lhs;
            node_t *rhs;
        } binary;

        struct ternary_t {
            node_t *cond;
            node_t *yes;
            node_t *no;
        } ternary;

        struct call_t {
            node_t *body;
            nodes_t *args;
        } call;

        struct func_t {
            char *name;
            nodes_t *params;
            node_t *result;
            node_t *body;
        } func;

        struct param_t {
            char *name;
            node_t *type;
        } param;

        node_t *expr;
        node_t *type;
        nodes_t *compound;
    };
} node_t;

nodes_t*
empty_node_list();

nodes_t*
new_node_list(node_t *init);

nodes_t*
node_append(nodes_t *self, node_t *node);

node_t*
new_digit(char *digit);

node_t*
new_unary(unary_op_t op, node_t *expr);

node_t*
new_binary(binary_op_t op, node_t *lhs, node_t *rhs);

node_t*
new_ternary(node_t *cond, node_t *yes, node_t *no);

node_t*
new_call(node_t *body, nodes_t *args);

node_t*
new_func(char *name, nodes_t *params, node_t *result, node_t *body);

node_t*
new_param(char *name, node_t *type);

node_t*
new_return(node_t *expr);

node_t*
new_name(char *name);

node_t*
new_typename(char *name);

node_t*
new_pointer(node_t *type);

node_t*
new_compound(nodes_t *nodes);

void
dump_node(node_t *node);

#endif /* AST_H */
