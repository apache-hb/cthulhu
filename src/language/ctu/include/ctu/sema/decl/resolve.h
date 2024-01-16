#pragma once

#include "ctu/ast.h"

typedef struct tree_t tree_t;

ctu_t *begin_resolve(tree_t *sema, tree_t *self, void *user, ctu_kind_t kind);

tree_t *ctu_cast_type(tree_t *sema, tree_t *expr, const tree_t *type);
