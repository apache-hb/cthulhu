#ifndef AST_H
#define AST_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct node_t node_t;

typedef enum {
    /* literals */
    NODE_DIGIT,
    NODE_STRING,
    NODE_NULL
} node_type_t;

typedef struct {
    size_t length;
    size_t size;
    struct node_t *data;
} nodes_t;

typedef struct node_t {
    node_type_t type;
    bool mut:1;
    bool exported:1;

    struct YYLTYPE *loc;

    union {
        char *text;

        struct digit_t {
            char *digit;
            char *suffix;
        } digit;
    };
} node_t;

node_t *new_digit(char *digit, char *suffix);
node_t *new_string(char *text);
node_t *new_null();
node_t *loc(node_t *node, struct YYLTYPE *loc);

#endif /* AST_H */
