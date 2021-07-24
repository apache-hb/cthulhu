#pragma once

#include "ctu/ast/ast.h"

extern type_t *VOID_TYPE;

typedef struct {
    list_t *nodes;
    size_t strings;
} unit_t;

unit_t typecheck(list_t *nodes);

void sema_init(void);
