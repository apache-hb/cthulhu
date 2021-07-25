#pragma once

#include "ctu/ast/ast.h"

typedef struct {
    list_t *decls;

    /**
     * total number of string literals
     */
    size_t strings;
} unit_t;

unit_t typecheck(node_t *root);

void sema_init(void);
