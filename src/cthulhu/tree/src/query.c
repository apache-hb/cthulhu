// SPDX-License-Identifier: LGPL-3.0-only

#include "arena/arena.h"
#include "base/util.h"
#include "common.h"

#include "cthulhu/tree/query.h"

#include "memory/memory.h"
#include "std/vector.h"
#include "std/str.h"

#include "base/panic.h"

#include <stdint.h>
#include <stdio.h>

static bool has_name(tree_kind_t kind)
{
    return kind_has_tag(kind, eTagName);
}

const char *tree_kind_to_string(tree_kind_t kind)
{
    switch (kind)
    {
#define TREE_KIND(ID, NAME, TAGS) case ID: return NAME;
#include "cthulhu/tree/tree.inc"

    default: CT_NEVER("invalid tree kind %d", kind);
    }
}

static const char *length_name(size_t length, arena_t *arena)
{
    if (length == SIZE_MAX) return "unbounded";
    return str_format(arena, "%zu", length);
}

const char *tree_to_string(const tree_t *self)
{
    arena_t *arena = get_global_arena();
    return tree_to_string_arena(self, arena);
}

char *tree_to_string_arena(const tree_t *self, arena_t *arena)
{
    CTASSERT(arena != NULL);

    if (self == NULL) { return arena_strdup("nil", arena); }

    tree_kind_t kind = tree_get_kind(self);
    switch (kind)
    {
    case eTreeError:
        return str_format(arena, "{ error: %s }", self->message);

    case eTreeTypeArray:
        return str_format(arena, "{ array %s { element: %s, length: %zu } }", tree_get_name(self), tree_to_string(self->ptr), self->length);

    case eTreeTypePointer:
        return str_format(arena, "{ pointer %s { to: %s, length: %s } }", tree_get_name(self), tree_to_string(self->ptr), length_name(self->length, arena));

    case eTreeTypeReference:
        return str_format(arena, "{ reference %s { to: %s } }", tree_get_name(self), tree_to_string(self->ptr));

    case eTreeTypeAlias:
        return str_format(arena, "{ alias %s { to: %s } }", tree_get_name(self), tree_to_string_arena(tree_get_type(self), arena));

    default:
        break;
    }

    if (has_name(self->kind))
    {
        return str_format(arena, "{ %s: %s }", tree_kind_to_string(self->kind), tree_get_name(self));
    }

    const char *it = tree_kind_to_string(self->kind);
    return arena_strdup(it, arena);
}

const tree_attribs_t *tree_get_attrib(const tree_t *self)
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

bool tree_has_vis(const tree_t *self, tree_visibility_t visibility)
{
    const tree_attribs_t *attrib = tree_get_attrib(self);
    return attrib->visibility == visibility;
}

///
/// quals
///

#if CTU_DEBUG
#   define EXPECT_STORAGE_DECL(SELF) CTASSERTF(tree_has_storage(SELF), "only globals and locals can have storage, got %s", tree_to_string(SELF))
#else
#   define EXPECT_STORAGE_DECL(SELF) (void)0
#endif

bool tree_has_storage(const tree_t *self)
{
    return tree_is(self, eTreeDeclGlobal) || tree_is(self, eTreeDeclLocal);
}

tree_storage_t get_storage(const tree_t *self)
{
    EXPECT_STORAGE_DECL(self);

    return self->storage;
}

tree_quals_t tree_get_storage_quals(const tree_t *self)
{
    tree_storage_t storage = get_storage(self);

    tree_quals_t quals = storage.quals;
    CTASSERTF((quals & (eQualConst | eQualMutable)) != (eQualConst | eQualMutable), "global %s has both const and mutable quals", tree_to_string(self));
    CTASSERTF(quals != eQualNone, "global %s has no quals", tree_to_string(self));
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

    CTASSERTF(storage.length != SIZE_MAX, "global %s has no storage length", tree_to_string(self));
    return storage.length;
}

///
/// enums
///

vector_t *tree_enum_get_cases(const tree_t *self)
{
    TREE_EXPECT(self, eTreeTypeEnum);

    return self->cases;
}

///
/// fns
///

const tree_t *tree_fn_get_return(const tree_t *self)
{
    switch (tree_get_kind(self))
    {
    case eTreeTypeReference: return tree_fn_get_return(self->ptr);
    case eTreeTypeClosure: return self->return_type;
    case eTreeDeclFunction: return tree_fn_get_return(tree_get_type(self));

    default: CT_NEVER("invalid function kind %s", tree_to_string(self));
    }
}

const vector_t *tree_fn_get_params(const tree_t *self)
{
    switch (tree_get_kind(self))
    {
    case eTreeTypeReference: return tree_fn_get_params(self->ptr);
    case eTreeTypeClosure: return self->params;
    case eTreeDeclFunction: return tree_fn_get_params(tree_get_type(self));

    default: CT_NEVER("invalid function kind %s", tree_to_string(self));
    }
}

tree_arity_t tree_fn_get_arity(const tree_t *self)
{
    switch (tree_get_kind(self))
    {
    case eTreeTypeClosure: return self->arity;
    case eTreeDeclFunction: {
        const tree_t *type = tree_get_type(self);
        return type->arity;
    }

    default: CT_NEVER("invalid function kind %s", tree_to_string(self));
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

tree_t *tree_ty_get_case(const tree_t *self, const char *name)
{
    TREE_EXPECT(self, eTreeTypeEnum);

    return find_field(self->cases, name);
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

static tree_quals_t get_quals(const tree_t *self)
{
    CTASSERT(self != NULL);
    return self->quals;
}

tree_quals_t tree_ty_get_quals(const tree_t *self)
{
    tree_quals_t quals = get_quals(self);

    // make sure we dont have both eQualConst and eQualMutable bits set
    CTASSERTF((quals & (eQualConst | eQualMutable)) != (eQualConst | eQualMutable), "type %s has both const and mutable quals", tree_to_string(self));

    return quals;
}

const tree_t *tree_ty_load_type(const tree_t *self)
{
    switch (tree_get_kind(self))
    {
    case eTreeTypeArray:
    case eTreeTypePointer:
    case eTreeTypeReference:
        return self->ptr;

    default:
        return self;
    }
}
