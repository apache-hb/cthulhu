// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

#include "cthulhu/tree/context.h"
#include "cthulhu/tree/query.h"
#include "cthulhu/tree/tree.h"

#include "base/panic.h"
#include <stdio.h>

static const tree_tags_t kTreeTags[eTreeTotal] = {
#define TREE_KIND(ID, NAME, TAGS) [ID] = (TAGS),
#include "cthulhu/tree/tree.inc"
};

bool kind_has_tag(tree_kind_t kind, tree_tags_t tags)
{
    CTASSERTF(kind < eTreeTotal, "invalid tree kind %d", kind);

    return kTreeTags[kind] & tags;
}

bool tree_has_tag(const tree_t *tree, tree_tags_t tags)
{
    CTASSERT(tree != NULL);
    return kind_has_tag(tree->kind, tags);
}

///
/// getters and setters
///

#if CTU_ASSERTS
static const char *tree_kind_string(const tree_t *tree)
{
    return tree_kind_to_string(tree_get_kind(tree));
}
#endif

STA_DECL
void tree_set_qualifiers(tree_t *tree, tree_quals_t qualifiers)
{
    CTASSERTF(tree_has_tag(tree, eTagQual), "tree type %s does not have qualifiers", tree_kind_string(tree));

    tree->quals = qualifiers;
}

STA_DECL
tree_quals_t tree_get_qualifiers(const tree_t *tree)
{
    CTASSERTF(tree_has_tag(tree, eTagQual), "tree type %s does not have qualifiers", tree_kind_string(tree));

    return tree->quals;
}

STA_DECL
void tree_set_storage(tree_t *tree, tree_storage_t storage)
{
    CTASSERTF(tree_has_tag(tree, eTagStorage), "tree type %s does not have storage", tree_kind_string(tree));
    if (storage.storage != NULL)
    {
        CTASSERTF(!tree_is(storage.storage, eTreeTypeReference), "storage cannot be a reference (%s)", tree_to_string(tree));
    }

    tree->storage = storage;
}

STA_DECL
tree_storage_t tree_get_storage(const tree_t *tree)
{
    CTASSERTF(tree_has_tag(tree, eTagStorage), "tree type %s does not have storage", tree_kind_string(tree));

    return tree->storage;
}

STA_DECL
void tree_set_eval(tree_t *tree, eval_model_t model)
{
    CTASSERTF(tree_has_tag(tree, eTagEval), "tree type %s does not have an evaluation model", tree_kind_string(tree));

    tree->eval_model = model;
}

STA_DECL
eval_model_t tree_get_eval(const tree_t *tree)
{
    CTASSERTF(tree_has_tag(tree, eTagEval), "tree type %s does not have an evaluation model", tree_kind_string(tree));

    return tree->eval_model;
}

STA_DECL
const node_t *tree_get_node(const tree_t *tree)
{
    CTASSERT(tree != NULL); // all trees have nodes

    return tree->node;
}

STA_DECL
const char *tree_get_name(const tree_t *tree)
{
    CTASSERTF(tree_has_tag(tree, eTagName), "tree type %s does not have a name", tree_kind_string(tree));

    if (tree_is(tree, eTreeError)) return tree->message;

    return tree->name;
}

STA_DECL
const char *tree_get_user_name(const tree_t *tree)
{
    CTASSERTF(tree_has_tag(tree, eTagName), "tree type %s does not have a symbol", tree_kind_string(tree));

    if (tree_is(tree, eTreeError)) return tree->message;

    return tree_is_symbol_anonymous(tree) ? "<anonymous>" : tree->name;
}

STA_DECL
bool tree_is_symbol_anonymous(const tree_t *tree)
{
    CTASSERTF(tree_has_tag(tree, eTagName), "tree type %s does not have a name", tree_kind_string(tree));

    if (tree_is(tree, eTreeError)) return false;

    return tree->name == NULL;
}

STA_DECL
const tree_t *tree_get_type(const tree_t *tree)
{
    CTASSERTF(tree_has_tag(tree, eTagHasType), "tree type %s does not have a type", tree_kind_string(tree));
    CTASSERTF(tree->type != NULL, "tree type %s has not been set", tree_kind_string(tree));

    if (tree_is(tree, eTreeError)) return tree;

    return tree->type;
}

STA_DECL
tree_kind_t tree_get_kind(const tree_t *tree)
{
    CTASSERT(tree != NULL);

    return tree->kind;
}

void tree_set_type(tree_t *self, const tree_t *type)
{
    CTASSERTF(tree_has_tag(self, eTagHasType), "tree type %s does not have a type", tree_kind_string(self));

    self->type = type;
}
