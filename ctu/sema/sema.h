#pragma once

#include "ctu/ast/ast.h"

#include "ctu/util/util.h"

extern const char *search_path;

typedef struct {
    nodes_t *decls;
    size_t strings;
} unit_t;

typedef struct sema_t {
    struct sema_t *parent;
    map_t *decls;
    map_t *imports;

    type_t *result; /* return type of current function */
} sema_t;

unit_t typecheck(node_t *root, sema_t **out);

void sema_init(void);
