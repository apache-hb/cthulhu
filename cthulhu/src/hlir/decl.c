#include "common.h"

#include "cthulhu/hlir/query.h"

#include "report/report.h"

#include "std/vector.h"

#include "base/panic.h"
#include "base/util.h"

#include <stdint.h>

static h2_t *decl_open(const node_t *node, const char *name, const h2_t *type, h2_kind_t expected, const h2_resolve_info_t *resolve)
{
    h2_t *self = h2_decl(expected, node, type, name);

    self->resolve = resolve;

    return self;
}

static void decl_close(h2_t *decl, h2_kind_t kind)
{
    CTASSERTF(h2_is(decl, kind), "decl %s is an unresolved %s, expected %s", h2_get_name(decl), h2_kind_to_string(decl->kind), h2_kind_to_string(kind));

    decl->kind = kind;
    decl->resolve = NULL;
}

h2_t *h2_resolve(h2_cookie_t *cookie, h2_t *decl)
{
    if (h2_is(decl, eHlir2Error)) { return decl; }

    const h2_resolve_info_t *res = decl->resolve;
    if (res == NULL) { return decl; }

    size_t index = vector_find(cookie->stack, decl);
    if (index != SIZE_MAX)
    {
        // TODO: better reporting
        report(cookie->reports, eFatal, decl->node, "cyclic dependency when resolving %s", h2_get_name(decl));
        return h2_error(decl->node, "cyclic dependency");
    }

    vector_push(&cookie->stack, decl);

    res->fnResolve(cookie, res->sema, decl, res->user);

    vector_drop(cookie->stack);

    return decl;
}

h2_t *h2_open_global(const node_t *node, const char *name, const h2_t *type, h2_resolve_info_t resolve)
{
    return decl_open(node, name, type, eHlir2DeclGlobal, BOX(resolve));
}

h2_t *h2_open_function(const node_t *node, const char *name, const h2_t *signature, h2_resolve_info_t resolve)
{
    h2_t *self = decl_open(node, name, signature, eHlir2DeclFunction, BOX(resolve));
    self->locals = vector_new(4);
    return self;
}

void h2_close_global(h2_t *self, h2_t *value)
{
    decl_close(self, eHlir2DeclGlobal);
    self->global = value;
}

void h2_close_function(h2_t *self, h2_t *body)
{
    CTASSERTF(h2_is(self, eHlir2DeclFunction), "decl %s is not a function", h2_get_name(self));

    decl_close(self, eHlir2DeclFunction);
    self->body = body;
}

h2_t *h2_decl_param(const node_t *node, const char *name, const h2_t *type)
{
    return h2_decl(eHlir2DeclParam, node, type, name);
}

h2_t *h2_decl_field(const node_t *node, const char *name, const h2_t *type)
{
    return h2_decl(eHlir2DeclField, node, type, name);
}

h2_t *h2_decl_local(const node_t *node, const char *name, const h2_t *type)
{
    return h2_decl(eHlir2DeclLocal, node, type, name);
}

h2_t *h2_open_decl(const node_t *node, const char *name, h2_resolve_info_t resolve)
{
    return decl_open(node, name, NULL, eHlir2Type, BOX(resolve));
}

void h2_close_decl(h2_t *self, const h2_t *other)
{
    CTASSERT(other != NULL);

    *self = *other;
}

h2_t *h2_decl_global(const node_t *node, const char *name, const h2_t *type, h2_t *value)
{
    h2_t *self = decl_open(node, name, type, eHlir2DeclGlobal, NULL);
    h2_close_global(self, value);
    return self;
}

h2_t *h2_decl_function(const node_t *node, const char *name, const h2_t *signature, vector_t *locals, h2_t *body)
{
    h2_t *self = decl_open(node, name, signature, eHlir2DeclFunction, NULL);
    self->locals = locals;
    h2_close_function(self, body);
    return self;
}

void h2_add_local(h2_t *self, h2_t *decl)
{
    CTASSERTF(h2_is(self, eHlir2DeclFunction), "cannot add locals to a declaration %s", h2_to_string(self));

    vector_push(&self->locals, decl);
}

void h2_set_attrib(h2_t *self, const h2_attrib_t *attrib)
{
    self->attrib = attrib;
}

///
/// structs
///

h2_t *h2_decl_struct(const node_t *node, const char *name, vector_t *fields)
{
    h2_t *self = decl_open(node, name, NULL, eHlir2TypeStruct, NULL);
    self->fields = fields;
    return self;
}

h2_t *h2_open_struct(const node_t *node, const char *name, h2_resolve_info_t resolve)
{
    h2_t *self = decl_open(node, name, NULL, eHlir2TypeStruct, BOX(resolve));
    self->fields = vector_new(4);
    return self;
}

void h2_close_struct(h2_t *self, vector_t *fields)
{
    CTASSERTF(h2_is(self, eHlir2TypeStruct), "decl %s is not a struct", h2_get_name(self));

    decl_close(self, eHlir2TypeStruct);
    self->fields = fields;
}
