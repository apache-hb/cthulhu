#include "common.h"

#include "cthulhu/tree/query.h"

#include "std/vector.h"
#include "std/str.h"

#include "base/panic.h"

#include <stdint.h>

static bool has_name(tree_kind_t kind)
{
    switch (kind)
    {
    case eTreeType:

    case eTreeTypeEmpty:
    case eTreeTypeUnit:
    case eTreeTypeBool:
    case eTreeTypeDigit:
    case eTreeTypeClosure:
    case eTreeTypePointer:
    case eTreeTypeReference:

    case eTreeTypeStruct:

    case eTreeDeclGlobal:
    case eTreeDeclLocal:
    case eTreeDeclParam:
    case eTreeDeclField:
    case eTreeDeclFunction:
    case eTreeDeclModule:
        return true;

    default:
        return false;
    }
}

const char *tree_kind_to_string(tree_kind_t kind)
{
    switch (kind)
    {
#define TREE_KIND(ID, NAME) case ID: return NAME;
#include "cthulhu/tree/tree.inc"

    default: NEVER("invalid tree kind %d", kind);
    }
}

const char *tree_to_string(const tree_t *self)
{
    if (self == NULL) { return "nil"; }

    if (tree_is(self, eTreeError))
    {
        return format("{ error: %s }", self->message);
    }

    if (has_name(self->kind))
    {
        return format("{ %s: %s }", tree_kind_to_string(self->kind), tree_get_name(self));
    }

    return tree_kind_to_string(self->kind);
}

const node_t *tree_get_node(const tree_t *self)
{
    CTASSERT(self != NULL);

    return self->node;
}

const char *tree_get_name(const tree_t *self)
{
    CTASSERT(self != NULL);
    if (tree_is(self, eTreeError)) { return self->message; }

    CTASSERTF(has_name(self->kind), "kind %s has no name", tree_kind_to_string(self->kind));

    return self->name;
}

tree_kind_t tree_get_kind(const tree_t *self)
{
    CTASSERT(self != NULL);

    return self->kind;
}

const tree_t *tree_get_type(const tree_t *self)
{
    if (tree_is(self, eTreeError)) { return self; }

    CTASSERTF(self->type != NULL, "missing type on %s", tree_to_string(self)); // type hasnt been set yet
    return self->type;
}

const attribs_t *tree_get_attrib(const tree_t *self)
{
    CTASSERT(self != NULL);
    CTASSERT(self->attrib != NULL);

    return self->attrib;
}

bool tree_is(const tree_t *self, tree_kind_t kind)
{
    CTASSERT(self != NULL);

    return self->kind == kind;
}

bool tree_has_vis(const tree_t *self, visibility_t visibility)
{
    const attribs_t *attrib = tree_get_attrib(self);
    return attrib->visibility == visibility;
}

///
/// quals
///

#define EXPECT_STORAGE_DECL(SELF) CTASSERTF(tree_is(SELF, eTreeDeclGlobal) || tree_is(SELF, eTreeDeclLocal), "only globals and locals can have storage, got %s", tree_to_string(SELF))

tree_storage_t get_storage(const tree_t *self)
{
    EXPECT_STORAGE_DECL(self);

    return self->storage;
}

quals_t tree_get_storage_quals(const tree_t *self)
{
    tree_storage_t storage = get_storage(self);

    quals_t quals = storage.quals;
    CTASSERTF((quals & (eQualConst | eQualMutable)) != (eQualConst | eQualMutable), "global %s has both const and mutable quals", tree_to_string(self));
    CTASSERTF(quals != eQualUnknown, "global %s has unknown quals", tree_to_string(self));
    return quals;
}

const tree_t *tree_get_storage_type(const tree_t *self)
{
    tree_storage_t storage = get_storage(self);

    CTASSERTF(storage.storage != NULL, "global %s has no storage type", tree_to_string(self));
    return storage.storage;
}

size_t tree_get_storage_size(const tree_t *self)
{
    tree_storage_t storage = get_storage(self);

    CTASSERTF(storage.size != SIZE_MAX, "global %s has no storage length", tree_to_string(self));
    return storage.size;
}

///
/// fns
///

const tree_t *tree_fn_get_return(const tree_t *self)
{
    switch (tree_get_kind(self))
    {
    case eTreeTypeClosure: return self->result;
    case eTreeDeclFunction: return self->type->result;

    default: NEVER("invalid function kind %s", tree_to_string(self));
    }
}

vector_t *tree_fn_get_params(const tree_t *self)
{
    switch (tree_get_kind(self))
    {
    case eTreeTypeClosure: return self->params;
    case eTreeDeclFunction: return self->type->params;

    default: NEVER("invalid function kind %s", tree_to_string(self));
    }
}

arity_t tree_fn_get_arity(const tree_t *self)
{
    switch (tree_get_kind(self))
    {
    case eTreeTypeClosure: return self->arity;
    case eTreeDeclFunction: return self->type->arity;

    default: NEVER("invalid function kind %s", tree_to_string(self));
    }
}

///
/// tys
///

static tree_t *find_field(vector_t *fields, const char *name)
{
    size_t len = vector_len(fields);
    for (size_t i = 0; i < len; i++)
    {
        tree_t *field = vector_get(fields, i);
        if (str_equal(tree_get_name(field), name))
        {
            return field;
        }
    }

    return NULL;
}

tree_t *tree_ty_get_field(const tree_t *self, const char *name)
{
    TREE_EXPECT(self, eTreeTypeStruct);

    return find_field(self->fields, name);
}

bool tree_ty_is_address(const tree_t *type)
{
    switch (tree_get_kind(type))
    {
    case eTreeTypePointer:
    case eTreeTypeReference:
        return true;

    default:
        return false;
    }
}

static quals_t get_quals(const tree_t *self)
{
    CTASSERT(self != NULL);
    return self->quals;
}

quals_t tree_ty_get_quals(const tree_t *self)
{
    quals_t quals = get_quals(self);

    // make sure we dont have both eQualConst and eQualMutable bits set
    CTASSERTF((quals & (eQualConst | eQualMutable)) != (eQualConst | eQualMutable), "type %s has both const and mutable quals", tree_to_string(self));

    return quals;
}

const tree_t *tree_ty_load_type(const tree_t *self)
{
    switch (tree_get_kind(self))
    {
    case eTreeTypePointer:
    case eTreeTypeReference:
        return self->ptr;

    default:
        return self;
    }
}
