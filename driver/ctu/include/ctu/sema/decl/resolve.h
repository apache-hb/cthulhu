#pragma once

#include "ctu/ast.h"

typedef struct tree_t tree_t;

ctu_t *begin_resolve(tree_t *sema, tree_t *self, void *user, ctu_kind_t kind);
