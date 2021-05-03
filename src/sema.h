#ifndef SEMA_H
#define SEMA_H

#include "ast.h"

typedef struct {
    /* all decls */
    nodes_t *decls;
    
    node_t *inttype;
    node_t *voidtype;
    node_t *booltype;
    /* number of errors */
    int errors;
} state_t;

void sema(state_t *self, node_t *program);

#endif /* SEMA_H */
