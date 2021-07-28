#pragma once

#include "ctu/ast/ast.h"

typedef struct {
    list_t *funcs;

    list_t *vars;

    list_t *types;

    /**
     * total number of string literals
     */
    size_t strings;
} unit_t;

unit_t typecheck(node_t *root);

void sema_init(void);
