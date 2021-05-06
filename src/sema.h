#ifndef SEMA_H
#define SEMA_H

#include "ast.h"

/* semantic analysis stuff */
typedef struct state_t {
    struct state_t *parent;
    
    /* all decls */
    nodes_t *decls;

    node_t *inttype;
    node_t *voidtype;
    node_t *booltype;
    node_t *chartype;

    node_t *ret;

    /* number of errors */
    int errors;
} state_t;

void sema(state_t *self, node_t *program);

#endif /* SEMA_H */
