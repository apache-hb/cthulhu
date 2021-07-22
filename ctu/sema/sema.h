#pragma once

#include "ctu/ast/ast.h"

extern type_t *VOID_TYPE;

typedef struct {
    node_t *root;
    size_t strings;
} unit_t;

unit_t typecheck(node_t *root);

void sema_init(void);
