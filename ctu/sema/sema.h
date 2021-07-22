#pragma once

#include "ctu/ast/ast.h"

extern type_t *VOID_TYPE;

typedef struct {
    nodes_t *nodes;
    size_t strings;
} unit_t;

unit_t typecheck(nodes_t *nodes);

void sema_init(void);
