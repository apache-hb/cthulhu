#pragma once

#include "ctu/ast/ast.h"

typedef struct sema_t {
    struct sema_t *parent;
    nodes_t *decls;
} sema_t;

void typecheck(nodes_t *nodes);

void sema_init(void);
