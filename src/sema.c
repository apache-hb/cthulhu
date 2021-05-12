#include "sema.h"

#include <stdio.h>
#include <string.h>

void sema_init(state_t *state)
{
    state->errors = 0;
    state->parent = NULL;
}

void nameresolve(state_t *state, node_t *node)
{
    (void)state;
    (void)node;
}

void typecheck(state_t *state, node_t *node)
{
    (void)state;
    (void)node;
}

void constcheck(state_t *state, node_t *node)
{
    (void)state;
    (void)node;
}
