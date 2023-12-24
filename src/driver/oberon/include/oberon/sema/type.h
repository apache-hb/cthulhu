#pragma once

#include "oberon/ast.h"

#include "cthulhu/tree/tree.h"

tree_t *obr_sema_type(tree_t *sema, obr_t *type, const char *name);

const tree_t *obr_rvalue_type(const tree_t *self);
