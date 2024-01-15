#pragma once

#include "cthulhu/tree/tree.h"

CT_LOCAL tree_t *tree_new(tree_kind_t kind, const node_t *node, const tree_t *type);
CT_LOCAL tree_t *tree_decl(tree_kind_t kind, const node_t *node, const tree_t *type, const char *name, quals_t quals);

#define TREE_EXPECT(SELF, KIND) CTASSERTF(tree_is(SELF, KIND), "expected %s, got %s", tree_kind_to_string(KIND), tree_to_string(SELF))
#define TREE_EXPECT_NOT(SELF, KIND) CTASSERTF(!tree_is(SELF, KIND), "expected not %s, got %s", tree_kind_to_string(KIND), tree_to_string(SELF))
