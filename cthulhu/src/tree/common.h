#pragma once

#include "cthulhu/tree/tree.h"

tree_t *tree_new(tree_kind_t kind, const node_t *node, const tree_t *type);
tree_t *tree_decl(tree_kind_t kind, const node_t *node, const tree_t *type, const char *name);

#define TREE_EXPECT(SELF, KIND) CTASSERTF(tree_is(SELF, KIND), "expected %s, got %s", tree_kind_to_string(KIND), tree_to_string(SELF))
