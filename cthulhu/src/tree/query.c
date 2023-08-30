#include "cthulhu/tree/query.h"

#include "std/vector.h"
#include "std/str.h"

#include "base/panic.h"

static bool has_name(tree_kind_t kind)
{
    switch (kind)
    {
    case eTreeType:

    case eTreeTypeEmpty:
    case eTreeTypeUnit:
    case eTreeTypeBool:
    case eTreeTypeDigit:
    case eTreeTypeString:
    case eTreeTypeClosure:
    case eTreeTypePointer:

    case eTreeTypeStruct:

    case eTreeDeclGlobal:
    case eTreeDeclLocal:
    case eTreeDeclParam:
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
    CTASSERT(self != NULL); // dont give me null

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

bool tree_has_quals(const tree_t *self, quals_t quals)
{
    if (tree_is(self, eTreeTypeQualify))
    {
        return self->quals & quals;
    }

    return false;
}

bool tree_has_vis(const tree_t *self, visibility_t visibility)
{
    const attribs_t *attrib = tree_get_attrib(self);
    return attrib->visibility == visibility;
}

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
