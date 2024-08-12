// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "ctu/sema/sema.h"

tree_t *ctu_sema_rvalue(ctu_sema_t *sema, const ctu_t *expr, const tree_t *implicit_type);
tree_t *ctu_sema_lvalue(ctu_sema_t *sema, const ctu_t *expr);

tree_t *ctu_sema_stmt(ctu_sema_t *sema, const ctu_t *stmt);

size_t ctu_resolve_storage_length(const tree_t *type);
const tree_t *ctu_resolve_storage_type(const tree_t *type);
const tree_t *ctu_resolve_decl_type(const tree_t *type);
