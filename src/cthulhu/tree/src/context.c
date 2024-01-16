#include "cthulhu/tree/context.h"
#include "cthulhu/tree/query.h"
#include "cthulhu/tree/tree.h"

#include "arena/arena.h"

#include "base/panic.h"
#include "std/str.h"
#include "std/set.h"

static const tree_attribs_t kDefaultAttrib = {
    .link = eLinkModule,
    .visibility = eVisiblePrivate
};

typedef struct tree_context_t
{
    arena_t *arena;

    set_t *trees;

    const char *name;
    tree_info_t info;
} tree_context_t;

typedef enum tree_tags_t
{
#define TREE_TAG(ID, NAME, TAGS) ID = (TAGS),
#include "cthulhu/tree/tree.def"
} tree_tags_t;

static const tree_tags_t kTreeTags[eTreeTotal] = {
#define TREE_KIND(ID, NAME, TAGS) [ID] = (TAGS),
#include "cthulhu/tree/tree.def"
};

static bool kind_has_tag(tree_kind_t kind, tree_tags_t tags)
{
    CTASSERTF(kind < eTreeTotal, "invalid tree kind %d", kind);

    return kTreeTags[kind] & tags;
}

static bool tree_has_tag(const tree_t *tree, tree_tags_t tags)
{
    CTASSERT(tree != NULL);
    return kind_has_tag(tree->kind, tags);
}

static tree_t *tree_new(tree_context_t *context, tree_kind_t kind, const node_t *node)
{
    CTASSERT(context != NULL);
    CTASSERT(node != NULL);

    tree_t *tree = ARENA_MALLOC(sizeof(tree_t), "tree", context, context->arena);
    tree->kind = kind;
    tree->node = node;
    tree->context = context;
    tree->type = NULL;

    return tree;
}

static tree_t *tree_named_new(tree_context_t *context, tree_kind_t kind, const node_t *node, const char *name)
{
    CTASSERT(context != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERTF(kind_has_tag(kind, eTagName), "attempted to name a kind %d with %s that cannot be named", kind, name);

    tree_t *tree = tree_new(context, kind, node);
    tree->decl_name = name;

    return tree;
}

static tree_t *tree_type_new(tree_context_t *context, tree_kind_t kind, const node_t *node, const char *name)
{
    CTASSERTF(kind_has_tag(kind, eTagQual), "attempted to create a type of %s named %s that cannot be qualified", tree_kind_to_string(kind), name);

    tree_t *tree = tree_named_new(context, kind, node, name);
    tree->attrib = &kDefaultAttrib;
    tree->resolve = NULL;
    tree_set_qualifiers(tree, eQualNone);
    return tree;
}

///
/// context management
///

static char *default_to_string(const tree_context_t *ctx, const tree_t *tree, arena_t *arena)
{
    CTASSERT(ctx != NULL);
    CTASSERT(tree != NULL);
    CTASSERT(arena != NULL);

    return tree_to_string_arena(tree, arena);
}

USE_DECL
tree_context_t *tree_context_new(tree_info_t info, const char *name, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(name != NULL);

    tree_info_t inner = info;
    if (inner.to_string == NULL) { inner.to_string = default_to_string; }

    tree_context_t *context = ARENA_MALLOC(sizeof(tree_context_t), name, NULL, arena);
    context->arena = arena;

    // TODO: hash consing once we generate the tree form
    context->trees = set_new(0x1000, kTypeInfoPtr, arena);

    context->name = name;
    context->info = inner;

    return context;
}

USE_DECL
void tree_context_delete(tree_context_t *context)
{
    CTASSERT(context != NULL);

    arena_free(context, sizeof(tree_context_t), context->arena);
}

bool tree_context_contains(const tree_context_t *context, const tree_t *tree)
{
    CTASSERT(context != NULL);
    CTASSERT(tree != NULL);

    return set_contains(context->trees, tree);
}

USE_DECL
char *tree_ctx_string(const tree_context_t *ctx, const tree_t *tree, arena_t *arena)
{
    CTASSERT(ctx != NULL);
    CTASSERT(tree != NULL);
    CTASSERT(arena != NULL);

    tree_info_t info = ctx->info;
    CTASSERT(info.to_string != NULL);

    if (tree == NULL)
        return arena_strdup("<null>", arena);

    if (tree_is(tree, eTreeError))
        return str_format(arena, "<error: `%s`>", tree->message);

    char *it = info.to_string(ctx, tree, arena);
    if (it != NULL) // TODO: should we decorate with the language name?
        return it;

    tree_kind_t kind = tree_get_kind(tree);
    const char *id = tree_kind_to_string(kind);

    if (tree_has_tag(tree, eTagName))
        return str_format(arena, "<%s: %s>", id, tree_get_name(tree));

    return str_format(arena, "<tree: %s>", id);
}

///
/// getters and setters
///

static const char *tree_kind_string(const tree_t *tree)
{
    return tree_kind_to_string(tree_get_kind(tree));
}

USE_DECL
void tree_set_qualifiers(tree_t *tree, quals_t qualifiers)
{
    CTASSERTF(tree_has_tag(tree, eTagQual), "tree type %s does not have qualifiers", tree_kind_string(tree));

    tree->quals = qualifiers;
}

USE_DECL
quals_t tree_get_qualifiers(const tree_t *tree)
{
    CTASSERTF(tree_has_tag(tree, eTagQual), "tree type %s does not have qualifiers", tree_kind_string(tree));

    return tree->quals;
}

USE_DECL
void tree_set_storage(tree_t *tree, tree_storage_t storage)
{
    CTASSERTF(tree_has_tag(tree, eTagStorage), "tree type %s does not have storage", tree_kind_string(tree));

    tree->decl_storage = storage;
}

USE_DECL
tree_storage_t tree_get_storage(const tree_t *tree)
{
    CTASSERTF(tree_has_tag(tree, eTagStorage), "tree type %s does not have storage", tree_kind_string(tree));

    return tree->decl_storage;
}

USE_DECL
const node_t *tree_get_node(const tree_t *tree)
{
    CTASSERT(tree != NULL); // all trees have nodes

    return tree->node;
}

USE_DECL
const char *tree_get_name(const tree_t *tree)
{
    CTASSERTF(tree_has_tag(tree, eTagName), "tree type %s does not have a name", tree_kind_string(tree));

    if (tree_is(tree, eTreeError)) return tree->message;

    return tree->decl_name;
}

USE_DECL
const tree_t *tree_get_type(const tree_t *tree)
{
    CTASSERTF(tree_has_tag(tree, eTagType), "tree type %s does not have a type", tree_kind_string(tree));
    CTASSERTF(tree->type != NULL, "tree type %s has not been set", tree_kind_string(tree));

    if (tree_is(tree, eTreeError)) return tree;

    return tree->type;
}

///
/// type constructors
///

USE_DECL
tree_t *tree_type_empty_new(tree_context_t *context, const node_t *node, const char *name)
{
    return tree_type_new(context, eTreeTypeEmpty, node, name);
}

USE_DECL
tree_t *tree_type_unit_new(tree_context_t *context, const node_t *node, const char *name)
{
    return tree_type_new(context, eTreeTypeUnit, node, name);
}

USE_DECL
tree_t *tree_type_bool_new(tree_context_t *context, const node_t *node, const char *name)
{
    return tree_type_new(context, eTreeTypeBool, node, name);
}

USE_DECL
tree_t *tree_type_opaque_new(tree_context_t *context, const node_t *node, const char *name)
{
    return tree_type_new(context, eTreeTypeOpaque, node, name);
}

USE_DECL
tree_t *tree_type_digit_new(tree_context_t *context, const node_t *node, const char *name, digit_t digit, sign_t sign)
{
    tree_t *tree = tree_type_new(context, eTreeTypeDigit, node, name);
    tree->digit = digit;
    tree->sign = sign;
    return tree;
}

USE_DECL
tree_t *tree_type_closure_new(tree_context_t *context, const node_t *node, const char *name, const tree_t *return_type, vector_t *params, arity_t arity)
{
    tree_t *tree = tree_type_new(context, eTreeTypeClosure, node, name);
    tree->return_type = return_type;
    tree->params = params;
    tree->arity = arity;
    return tree;
}

USE_DECL
tree_t *tree_type_pointer_new(tree_context_t *context, const node_t *node, const char *name, const tree_t *pointee, const tree_t *length)
{
    tree_t *tree = tree_type_new(context, eTreeTypePointer, node, name);
    tree->ptr = pointee;
    tree->len = length;
    return tree;
}

USE_DECL
tree_t *tree_type_array_new(tree_context_t *context, const node_t *node, const char *name, const tree_t *element, const tree_t *length)
{
    tree_t *tree = tree_type_new(context, eTreeTypeArray, node, name);
    tree->ptr = element;
    tree->len = length;
    return tree;
}

USE_DECL
tree_t *tree_type_reference_new(tree_context_t *context, const node_t *node, const char *name, const tree_t *pointee)
{
    tree_t *tree = tree_type_new(context, eTreeTypeReference, node, name);
    tree->ptr = pointee;
    return tree;
}

///
/// expressions
///

USE_DECL
tree_t *tree_expr_empty_new(tree_context_t *context, const node_t *node, const tree_t *type)
{
    tree_t *tree = tree_new(context, eTreeExprEmpty, node);
    tree->type = type;
    return tree;
}

USE_DECL
tree_t *tree_expr_unit_new(tree_context_t *context, const node_t *node, const tree_t *type)
{
    tree_t *tree = tree_new(context, eTreeExprUnit, node);
    tree->type = type;
    return tree;
}
