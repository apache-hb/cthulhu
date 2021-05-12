#ifndef SEMA_H
#define SEMA_H

#include "ast.h"

typedef struct state_t {
    struct state_t *parent;
    int errors;

    nodes_t *decls;
} state_t;

void sema_init(state_t *state);

/* resolve typenames */
void nameresolve(state_t *state, node_t *node);

/* check typecasting and validate assignments */
void typecheck(state_t *state, node_t *node);

/* enforce compile time checks */
void constcheck(state_t *state, node_t *node);

#endif /* SEMA_H */
