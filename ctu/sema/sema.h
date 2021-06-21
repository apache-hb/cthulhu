#pragma once

#include "ctu/ast/ast.h"

typedef struct sema_t {
    struct sema_t *parent;
    nodes_t *nodes;

    nodes_t *decls;
} sema_t;

sema_t *resolve(nodes_t *nodes);

void typecheck(sema_t *sema);
