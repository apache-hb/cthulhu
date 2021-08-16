#include "sema.h"

#include <stdbool.h>

sema_t *sema_new(sema_t *parent) {
    sema_t *sema = ctu_malloc(sizeof(sema_t));
    sema->parent = parent;
    sema->locals = vector_new(4);
    return sema;    
}

void sema_delete(sema_t *sema) {
    vector_delete(sema->locals);
    ctu_free(sema);
}

static bool local_eq(node_t *local, node_t *name) {
    return (strcmp(local->name->ident, name->ident) == 0);
}

node_t *sema_get(sema_t *sema, node_t *name) {
    vector_t *locals = sema->locals;
    for (size_t i = 0; i < vector_len(locals); i++) {
        node_t *local = vector_get(locals, i);
        if (local_eq(local, name)) {
            return local;
        }
    }

    if (sema->parent != NULL) {
        return sema_get(sema->parent, name);
    }

    return NULL;
}
