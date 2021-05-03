#include "sema.h"

#include <stdio.h>

#define IS(node, type) if (node == NULL) { fprintf(stderr, "node was NULL\n"); return; } if (node->kind != type) { fprintf(stderr, "incorrect node type\n"); return; }

static void
sema_func(struct func_t *func)
{
    (void)func;
}

void 
sema(node_t *program)
{
    IS(program, NODE_COMPOUND);

    for (size_t i = 0; i < program->compound->length; i++) {
        sema_func(&(program->compound->data + i)->func);
    }
}
