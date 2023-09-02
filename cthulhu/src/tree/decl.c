#include "common.h"

#include "cthulhu/tree/query.h"

#include "report/report.h"

#include "std/vector.h"

#include "base/panic.h"
#include "base/util.h"

#include <stdint.h>

static tree_t *decl_open(const node_t *node, const char *name, const tree_t *type, tree_kind_t expected, const tree_resolve_info_t *resolve)
{
    tree_t *self = tree_decl(expected, node, type, name, eQualUnknown);

    if (resolve != NULL)
    {
        CTASSERT(resolve->fnResolve != NULL);
        CTASSERT(resolve->sema != NULL);
    }

    self->resolve = resolve;

    return self;
}

static void decl_close(tree_t *decl, tree_kind_t kind)
{
    CTASSERTF(tree_is(decl, kind), "decl %s is an unresolved %s, expected %s", tree_get_name(decl), tree_kind_to_string(decl->kind), tree_kind_to_string(kind));

    decl->kind = kind;
    decl->resolve = NULL;
}

tree_t *tree_resolve(cookie_t *cookie, const tree_t *decl)
{
    tree_t *inner = (tree_t*)decl;
    if (tree_is(decl, eTreeError)) { return inner; }

    const tree_resolve_info_t *res = decl->resolve;
    if (res == NULL) { return inner; }

    size_t index = vector_find(cookie->stack, decl);
    if (index != SIZE_MAX)
    {
        // TODO: better reporting
        report(cookie->reports, eFatal, decl->node, "cyclic dependency when resolving %s", tree_get_name(decl));
        return tree_error(decl->node, "cyclic dependency");
    }

    vector_push(&cookie->stack, inner);

    res->fnResolve(cookie, res->sema, inner, res->user);

    vector_drop(cookie->stack);

    return inner;
}


tree_t *tree_decl_global(const node_t *node, const char *name, const tree_t *type, tree_t *value)
{
    tree_t *self = decl_open(node, name, type, eTreeDeclGlobal, NULL);
    tree_close_global(self, value);
    return self;
}

tree_t *tree_open_global(const node_t *node, const char *name, const tree_t *type, tree_resolve_info_t resolve)
{
    return decl_open(node, name, type, eTreeDeclGlobal, BOX(resolve));
}

tree_t *tree_open_function(const node_t *node, const char *name, const tree_t *signature, tree_resolve_info_t resolve)
{
    CTASSERTF(tree_is(signature, eTreeTypeClosure), "signature %s is not a closure", tree_to_string(signature));

    tree_t *self = decl_open(node, name, signature, eTreeDeclFunction, BOX(resolve));
    self->params = signature->params;
    self->locals = vector_new(4);
    return self;
}

void tree_close_global(tree_t *self, tree_t *value)
{
    decl_close(self, eTreeDeclGlobal);
    self->global = value;
}

void tree_close_function(tree_t *self, tree_t *body)
{
    TREE_EXPECT(self, eTreeDeclFunction);
    TREE_EXPECT(tree_get_type(self), eTreeTypeClosure);

    decl_close(self, eTreeDeclFunction);
    self->body = body;

    CTASSERTF(vector_len(self->params) == vector_len(self->type->params),
        "decl %s has %zu params, expected %zu parameter(s)",
        tree_get_name(self), vector_len(self->params),
        vector_len(self->type->params), (self->type->arity == eArityFixed) ? "" : " or more"
    );
}

tree_t *tree_decl_param(const node_t *node, const char *name, const tree_t *type)
{
    return tree_decl(eTreeDeclParam, node, type, name, eQualUnknown);
}

tree_t *tree_decl_field(const node_t *node, const char *name, const tree_t *type)
{
    return tree_decl(eTreeDeclField, node, type, name, eQualUnknown);
}

tree_t *tree_decl_local(const node_t *node, const char *name, const tree_t *type)
{
    return tree_decl(eTreeDeclLocal, node, type, name, eQualUnknown);
}

tree_t *tree_open_decl(const node_t *node, const char *name, tree_resolve_info_t resolve)
{
    return decl_open(node, name, NULL, eTreeType, BOX(resolve));
}

void tree_close_decl(tree_t *self, const tree_t *other)
{
    CTASSERT(other != NULL);

    *self = *other;
}

tree_t *tree_decl_function(
    const node_t *node, const char *name, const tree_t *signature,
    vector_t *params, vector_t *locals, tree_t *body
)
{
    tree_t *self = decl_open(node, name, signature, eTreeDeclFunction, NULL);
    self->params = params;
    self->locals = locals;
    tree_close_function(self, body);
    return self;
}

void tree_add_param(tree_t *self, tree_t *decl)
{
    CTASSERTF(tree_is(self, eTreeDeclFunction), "cannot add params to a declaration %s", tree_to_string(self));
    CTASSERTF(tree_is(decl, eTreeDeclParam), "cannot add a non-param %s to a function as a param", tree_to_string(decl));

    vector_push(&self->params, decl);
}

void tree_add_local(tree_t *self, tree_t *decl)
{
    CTASSERTF(tree_is(self, eTreeDeclFunction), "cannot add locals to a declaration %s", tree_to_string(self));
    CTASSERTF(tree_is(decl, eTreeDeclLocal), "cannot add a non-local %s to a function as a local", tree_to_string(decl));

    vector_push(&self->locals, decl);
}

void tree_set_attrib(tree_t *self, const attribs_t *attrib)
{
    self->attrib = attrib;
}

tree_t *tree_alias(const tree_t *tree, const char *name)
{
    CTASSERTF(tree != NULL && name != NULL, "(tree=%p, name=%p)", tree, name);

    tree_t *copy = ctu_memdup(tree, sizeof(tree_t));
    copy->name = name;
    return copy;
}

///
/// structs
///

tree_t *tree_decl_struct(const node_t *node, const char *name, vector_t *fields)
{
    tree_t *self = decl_open(node, name, NULL, eTreeTypeStruct, NULL);
    self->fields = fields;
    return self;
}

tree_t *tree_open_struct(const node_t *node, const char *name, tree_resolve_info_t resolve)
{
    tree_t *self = decl_open(node, name, NULL, eTreeTypeStruct, BOX(resolve));
    self->fields = vector_new(4);
    return self;
}

void tree_close_struct(tree_t *self, vector_t *fields)
{
    decl_close(self, eTreeTypeStruct);
    self->fields = fields;
}
