// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "oberon/ast.h"

#include "cthulhu/tree/tree.h"

tree_t *obr_sema_rvalue(tree_t *sema, obr_t *expr, const tree_t *implicit_type);
tree_t *obr_sema_lvalue(tree_t *sema, obr_t *expr);

tree_t *obr_default_value(const node_t *node, const tree_t *type);

tree_t *obr_sema_stmts(tree_t *sema, const node_t *node, vector_t *stmts);
