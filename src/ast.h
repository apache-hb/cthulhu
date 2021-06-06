#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define ARRAY_IMPL(array, type, add_name, at_name, new_name, grow) \
    static size_t add_name(array *self, type it) { \
        if (self->len + 1 >= self->size) { \
            self->size += grow; \
            self->data = realloc(self->data, sizeof(type) * self->size); \
        } \
        self->data[self->len] = it; \
        return self->len++; \
    } \
    type *at_name(array *self, size_t idx) { \
        if (idx > self->len) { \
            fprintf(stderr, #at_name "(%zu > %zu)\n", idx, self->len); \
        } \
        return self->data + idx; \
    } \
    static array new_name(void) { \
        array self = { malloc(sizeof(type) * grow), grow, 0 }; \
        return self; \
    }

typedef enum {
    NODE_DIGIT,
    NODE_SYMBOL,

    NODE_BINARY,
    NODE_UNARY,
    NODE_TERNARY,
    NODE_CALL,

    NODE_RETURN
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

        node_t *expr;

        struct call_t {
            node_t *expr;
            nodes_t *args;
        } call; 
    };
} node_t;

nodes_t *ast_empty(void);
nodes_t *ast_list(node_t *init);
nodes_t *ast_append(nodes_t *list, node_t *item);

node_t *ast_symbol(char *text);
node_t *ast_digit(char *text);
node_t *ast_binary(node_t *lhs, node_t *rhs, int op);
node_t *ast_unary(node_t *expr, int op);
node_t *ast_ternary(node_t *cond, node_t *lhs, node_t *rhs);
node_t *ast_return(node_t *expr);
node_t *ast_call(node_t *expr, nodes_t *args);
