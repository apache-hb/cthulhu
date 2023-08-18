#pragma once

#include "cthulhu/tree/tree.h"

tree_t *tree_new(tree_kind_t kind, const node_t *node, const tree_t *type);
tree_t *tree_decl(tree_kind_t kind, const node_t *node, const tree_t *type, const char *name);
