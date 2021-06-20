#pragma once

#include "ctu/ast/ast.h"

typedef struct sema_t {
    struct sema_t *parent;
} sema_t;

void resolve(nodes_t *nodes);

void typecheck(nodes_t *nodes);
