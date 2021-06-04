#include "sema.h"
#include <stdio.h>

nodes_t *world;

static node_t *find_symbol(const char *name) {
    /* for now dont resolve anything */
    (void)name;
    return NULL;
}

static void resolve_node(node_t *node) {
    size_t i = 0;

    switch (node->type) {
    case NODE_RETURN: 
        resolve_node(node->expr);
        break;

    case NODE_UNARY:
        resolve_node(node->unary.expr);
        break;

    case NODE_BINARY:
        resolve_node(node->binary.lhs);
        resolve_node(node->binary.rhs);
        break;

    case NODE_CALL:
        for (; i < node->call.args->len; i++)
            resolve_node(node->call.args->data + i);
        resolve_node(node->call.expr);
        break;

    case NODE_TERNARY:
        resolve_node(node->ternary.cond);
        resolve_node(node->ternary.lhs);
        resolve_node(node->ternary.rhs);
        break;

    case NODE_DIGIT:
        break;

    case NODE_SYMBOL:
        if (find_symbol(node->text) == NULL) {
            fprintf(stderr, "failed to find symbol %s\n", node->text);
        }
        break;
    }
}

void sym_resolve(nodes_t *all) {
    world = all;

    for (size_t i = 0; i < world->len; i++)
        resolve_node(world->data + i);
}   
