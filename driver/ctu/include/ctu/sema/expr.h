#pragma once

#include "ctu/sema/sema.h"

tree_t *ctu_sema_rvalue(ctu_sema_t *sema, const ctu_t *expr, const tree_t *implicitType);
tree_t *ctu_sema_lvalue(ctu_sema_t *sema, const ctu_t *expr);

tree_t *ctu_sema_stmt(ctu_sema_t *sema, const ctu_t *stmt);

size_t ctu_resolve_storage_size(const tree_t *type);
const tree_t *ctu_resolve_storage_type(const tree_t *type);
const tree_t *ctu_resolve_decl_type(const tree_t *type);
