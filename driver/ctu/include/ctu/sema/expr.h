#pragma once

#include "ctu/sema/sema.h"

tree_t *ctu_sema_rvalue(tree_t *sema, const ctu_t *expr, const tree_t *implicitType);
tree_t *ctu_sema_lvalue(tree_t *sema, const ctu_t *expr, const tree_t *implicitType);

tree_t *ctu_sema_stmt(tree_t *sema, tree_t *decl, const ctu_t *stmt);

size_t ctu_resolve_storage_size(const tree_t *type);
