#pragma once

#include "ctu/ast/ast.h"

extern type_t *VOID_TYPE;

typedef struct {
    list_t *decls;
    size_t strings;
} unit_t;

unit_t typecheck(node_t *root);

void sema_init(void);
