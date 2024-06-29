// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "cthulhu/tree/tree.h"

typedef enum tree_tags_t
{
#define TREE_TAG(ID, NAME, TAGS) ID = (TAGS),
#include "cthulhu/tree/tree.inc"
} tree_tags_t;

CT_CONSTFN CT_LOCAL
bool kind_has_tag(tree_kind_t kind, tree_tags_t tags);

CT_CONSTFN CT_LOCAL
bool tree_has_tag(const tree_t *tree, tree_tags_t tags);

CT_LOCAL tree_t *tree_new(tree_kind_t kind, const node_t *node, const tree_t *type);
CT_LOCAL tree_t *tree_decl(tree_kind_t kind, const node_t *node, const tree_t *type, const char *name, tree_quals_t quals);

#define TREE_EXPECT(SELF, KIND) CTASSERTF(tree_is(SELF, KIND), "expected %s, got %s", tree_kind_to_string(KIND), tree_to_string(SELF))
#define TREE_EXPECT_NOT(SELF, KIND) CTASSERTF(!tree_is(SELF, KIND), "expected not %s, got %s", tree_kind_to_string(KIND), tree_to_string(SELF))
