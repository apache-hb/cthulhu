// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

#include "core/macros.h"
#include "cthulhu/events/events.h"
#include "cthulhu/tree/query.h"

#include "std/vector.h"

#include "memory/memory.h"
#include "arena/arena.h"

#include "base/panic.h"
#include <stdint.h>
#include <stdio.h>

static const tree_storage_t kEmptyStorage = {
    .storage = NULL,
    .length = SIZE_MAX,
    .quals = eQualNone
};

static tree_t *decl_open(const node_t *node, const char *name, const tree_t *type, tree_kind_t expected, const tree_resolve_info_t *resolve)
{
    tree_t *self = tree_decl(expected, node, type, name, eQualNone);

    if (resolve != NULL)
    {
        CTASSERT(resolve->fn_resolve != NULL);
        CTASSERT(resolve->sema != NULL);
    }

    self->resolve = resolve;

    return self;
}

static void decl_close(tree_t *decl, tree_kind_t kind)
{
    CTASSERTF(tree_is(decl, kind), "decl %s is the wrong type, expected %s", tree_to_string(decl), tree_kind_to_string(kind));

    decl->kind = kind;
    decl->resolve = NULL;
}

tree_t *tree_resolve(tree_cookie_t *cookie, const tree_t *decl)
{
    tree_t *inner = (tree_t*)decl;
    if (tree_is(decl, eTreeError)) { return inner; }

    const tree_resolve_info_t *res = decl->resolve;
    if (res == NULL) { return inner; }

    size_t index = vector_find(cookie->stack, decl);
    if (index != SIZE_MAX)
    {
        // TODO: better reporting
        msg_notify(cookie->reports, &kEvent_CyclicDependency, decl->node, "cyclic dependency when resolving %s", tree_get_name(decl));
        return tree_error(decl->node, &kEvent_CyclicDependency, "cyclic dependency");
    }

    vector_push(&cookie->stack, inner);

    CTASSERTF(res->fn_resolve != NULL, "resolve function for %s is NULL", tree_to_string(inner));
    res->fn_resolve(res->sema, inner, res->user);

    vector_drop(cookie->stack);

    return inner;
}

static tree_t *resolve_type_inner(const tree_t *decl)
{
    tree_t *inner = (tree_t*)decl;
    if (tree_is(decl, eTreeError)) { return inner; }

    if (tree_is(decl, eTreePartial))
    {
        const tree_resolve_info_t *res = decl->resolve;
        CTASSERTF(res != NULL, "resolve info for %s is NULL", tree_to_string(decl));
        CTASSERTF(res->fn_resolve_type != NULL, "resolve type function for %s is NULL", tree_to_string(decl));

        res->fn_resolve_type(res->sema, inner, res->user);

        return inner;
    }

    if (!tree_is(decl, eTreeDeclFunction)) { return inner; }

    const tree_resolve_info_t *res = decl->resolve;
    if (res == NULL) { return inner; }

    // TODO: should this be ok?
    if (res->fn_resolve_type == NULL) { return inner; }

    res->fn_resolve_type(res->sema, inner, res->user);

    return inner;
}

tree_t *tree_resolve_type(tree_cookie_t *cookie, const tree_t *decl)
{
    CTASSERT(cookie != NULL);

    size_t index = vector_find(cookie->types, decl);
    if (index != SIZE_MAX)
    {
        // TODO: better reporting
        msg_notify(cookie->reports, &kEvent_CyclicDependency, decl->node, "cyclic dependency when resolving %s", tree_get_name(decl));
        return tree_error(decl->node, &kEvent_CyclicDependency, "cyclic dependency");
    }

    vector_push(&cookie->types, (tree_t*)decl);

    tree_t *result = resolve_type_inner(decl);

    vector_drop(cookie->types);

    return result;
}

tree_t *tree_decl_global(
    const node_t *node, const char *name,
    tree_storage_t storage, const tree_t *type, tree_t *value)
{
    tree_t *self = decl_open(node, name, type, eTreeDeclGlobal, NULL);
    tree_set_storage(self, storage);
    tree_close_global(self, value);
    return self;
}

static tree_resolve_info_t *dup_resolve_info(tree_resolve_info_t *info)
{
    arena_t *arena = get_global_arena();
    tree_resolve_info_t *copy = arena_memdup(info, sizeof(tree_resolve_info_t), arena);
    return copy;
}

tree_t *tree_open_global(const node_t *node, const char *name, const tree_t *type, tree_resolve_info_t resolve)
{
    tree_t *self = decl_open(node, name, type, eTreeDeclGlobal, dup_resolve_info(&resolve));
    tree_set_storage(self, kEmptyStorage);
    return self;
}

tree_t *tree_open_function(const node_t *node, const char *name, const tree_t *signature, tree_resolve_info_t resolve)
{
    if (signature != NULL)
    {
        CTASSERTF(tree_is(signature, eTreeTypeClosure), "signature %s is not a closure", tree_to_string(signature));
    }

    arena_t *arena = get_global_arena();

    tree_t *self = decl_open(node, name, signature, eTreeDeclFunction, dup_resolve_info(&resolve));
    self->params = signature == NULL ? NULL : signature->params;
    self->locals = vector_new(4, arena);
    return self;
}

void tree_close_global(tree_t *self, tree_t *value)
{
    decl_close(self, eTreeDeclGlobal);
    self->initial = value;
}

void tree_close_function(tree_t *self, tree_t *body)
{
    TREE_EXPECT(self, eTreeDeclFunction);
    TREE_EXPECT(tree_get_type(self), eTreeTypeClosure);

    decl_close(self, eTreeDeclFunction);
    self->body = body;

    const vector_t *params = tree_fn_get_params(self);
    tree_arity_t arity = tree_fn_get_arity(self);

    CT_UNUSED(arity);

    CTASSERTF(vector_len(self->params) == vector_len(params),
        "decl %s has %zu params, expected %zu%s parameter(s)",
        tree_get_name(self), vector_len(self->params),
        vector_len(params), (arity == eArityFixed) ? "" : " or more"
    );
}

tree_t *tree_decl_param(const node_t *node, const char *name, const tree_t *type)
{
    return tree_decl(eTreeDeclParam, node, type, name, eQualNone);
}

tree_t *tree_decl_field(const node_t *node, const char *name, const tree_t *type)
{
    return tree_decl(eTreeDeclField, node, type, name, eQualNone);
}

tree_t *tree_decl_local(const node_t *node, const char *name, tree_storage_t storage, const tree_t *type)
{
    tree_t *self = tree_decl(eTreeDeclLocal, node, type, name, eQualNone);
    tree_set_storage(self, storage);
    return self;
}

tree_t *tree_decl_case(const node_t *node, const char *name, tree_t *expr)
{
    tree_t *self = tree_decl(eTreeDeclCase, node, tree_get_type(expr), name, eQualNone);
    self->case_value = expr;
    return self;
}

tree_t *tree_open_decl(const node_t *node, const char *name, tree_resolve_info_t resolve)
{
    return decl_open(node, name, NULL, eTreePartial, dup_resolve_info(&resolve));
}

void tree_close_decl(tree_t *self, const tree_t *other)
{
    CTASSERT(other != NULL);

    *self = *other;
}

tree_t *tree_decl_function(
    const node_t *node, const char *name, const tree_t *signature,
    const vector_t *params, vector_t *locals, tree_t *body
)
{
    tree_t *self = decl_open(node, name, signature, eTreeDeclFunction, NULL);
    self->params = params;
    self->locals = locals;
    tree_close_function(self, body);
    return self;
}

tree_t *tree_decl_attrib(const node_t *node, const char *name, vector_t *params)
{
    tree_t *self = tree_decl(eTreeDeclAttrib, node, NULL, name, eQualNone);
    self->params = params;
    return self;
}

void tree_add_local(tree_t *self, tree_t *decl)
{
    CTASSERTF(tree_is(self, eTreeDeclFunction), "cannot add locals to a declaration %s", tree_to_string(self));
    CTASSERTF(tree_is(decl, eTreeDeclLocal), "cannot add a non-local %s to a function as a local", tree_to_string(decl));

    vector_push(&self->locals, decl);
}

void tree_set_attrib(tree_t *self, const tree_attribs_t *attrib)
{
    CTASSERT(self != NULL);

    self->attrib = attrib;
}

tree_t *tree_alias(const tree_t *tree, const char *name)
{
    CTASSERTF(tree != NULL && name != NULL, "(tree=%p, name=%p)", (void*)tree, (void*)name);

    arena_t *arena = get_global_arena();
    tree_t *copy = arena_memdup(tree, sizeof(tree_t), arena);
    copy->name = name;
    return copy;
}

tree_t *tree_type_alias(const node_t *node, const char *name, const tree_t *type, tree_quals_t quals)
{
    tree_t *self = tree_decl(eTreeTypeAlias, node, type, name, quals);
    return self;
}

const tree_t *tree_follow_type(const tree_t *type)
{
    if (type == NULL) return NULL;

    if (tree_is(type, eTreeTypeAlias))
        return tree_follow_type(tree_get_type(type));

    return type;
}

///
/// structs
///

static void check_aggregate_fields(vector_t *fields)
{
    size_t len = vector_len(fields);
    for (size_t i = 0; i < len; i++)
    {
        const tree_t *field = vector_get(fields, i);
        CTASSERTF(tree_is(field, eTreeDeclField), "expected field, got %s", tree_to_string(field));
    }
}

tree_t *tree_decl_struct(const node_t *node, const char *name, vector_t *fields)
{
    check_aggregate_fields(fields);

    tree_t *self = decl_open(node, name, NULL, eTreeTypeStruct, NULL);
    self->fields = fields;
    return self;
}

tree_t *tree_open_struct(const node_t *node, const char *name, tree_resolve_info_t resolve)
{
    arena_t *arena = get_global_arena();
    tree_t *self = decl_open(node, name, NULL, eTreeTypeStruct, dup_resolve_info(&resolve));
    self->fields = vector_new(4, arena);
    return self;
}

void tree_close_struct(tree_t *self, vector_t *fields)
{
    decl_close(self, eTreeTypeStruct);
    self->fields = fields;
    check_aggregate_fields(fields);
}

///
/// unions
///

tree_t *tree_decl_union(const node_t *node, const char *name, vector_t *fields)
{
    check_aggregate_fields(fields);

    tree_t *self = decl_open(node, name, NULL, eTreeTypeUnion, NULL);
    self->fields = fields;
    return self;
}

tree_t *tree_open_union(const node_t *node, const char *name, tree_resolve_info_t resolve)
{
    arena_t *arena = get_global_arena();
    tree_t *self = decl_open(node, name, NULL, eTreeTypeUnion, dup_resolve_info(&resolve));
    self->fields = vector_new(4, arena);
    return self;
}

void tree_close_union(tree_t *self, vector_t *fields)
{
    decl_close(self, eTreeTypeUnion);
    self->fields = fields;
    check_aggregate_fields(fields);
}

///
/// enums
///

#define CHECK_CASE(TY) CTASSERTF(tree_is(TY, eTreeDeclCase), "expected case, got %s", tree_to_string(TY));

static void check_enum_cases(vector_t *cases)
{
    size_t len = vector_len(cases);
    for (size_t i = 0; i < len; i++)
    {
        const tree_t *field = vector_get(cases, i);
        CHECK_CASE(field);
    }
}

tree_t *tree_decl_enum(const node_t *node, const char *name, const tree_t *underlying, vector_t *cases, tree_t *default_case)
{
    tree_t *self = decl_open(node, name, NULL, eTreeTypeEnum, NULL);
    tree_close_enum(self, underlying, cases, default_case);
    return self;
}

tree_t *tree_open_enum(const node_t *node, const char *name, tree_resolve_info_t resolve)
{
    arena_t *arena = get_global_arena();
    tree_t *self = decl_open(node, name, NULL, eTreeTypeEnum, dup_resolve_info(&resolve));
    self->underlying = NULL;
    self->cases = vector_new(4, arena);
    self->default_case = NULL;
    return self;
}

void tree_close_enum(tree_t *self, const tree_t *underlying, vector_t *cases, tree_t *default_case)
{
    decl_close(self, eTreeTypeEnum);

    self->underlying = underlying;
    self->cases = cases;

    // TODO: should this be a responsibility of tree?
    self->default_case = default_case;

    CTASSERTF(tree_is(underlying, eTreeTypeDigit), "enums must have an underlying digit type, got %s", tree_to_string(underlying));

    check_enum_cases(cases);
    if (default_case != NULL)
    {
        CHECK_CASE(default_case);
    }
}
