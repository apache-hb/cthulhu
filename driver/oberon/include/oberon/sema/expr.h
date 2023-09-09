#pragma once

#include "oberon/sema/sema.h"

tree_t *obr_sema_rvalue(tree_t *sema, obr_t *expr, const tree_t *implicitType);
tree_t *obr_sema_lvalue(tree_t *sema, obr_t *expr);

tree_t *obr_default_value(const node_t *node, const tree_t *type);

tree_t *obr_sema_stmts(tree_t *sema, const node_t *node, vector_t *stmts);
