#pragma once

#include "ctu/util/util.h"

typedef struct sema_t {
    struct sema_t *parent;

    vector_t *locals;
} sema_t;

sema_t *sema_new(sema_t *parent);
void sema_delete(sema_t *sema);

node_t *sema_get(sema_t *sema, node_t *name);
