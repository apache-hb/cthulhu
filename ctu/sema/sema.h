#pragma once

#include "ctu/ast/ast.h"

typedef struct {
    list_t *funcs;

    list_t *vars;

    list_t *types;

    /* required libraries to link to */
    list_t *libs;

    /* required C headers for interop */
    list_t *headers;

    /**
     * total number of string literals
     */
    size_t strings;
} unit_t;

unit_t typecheck(node_t *root);

void sema_init(void);
