#include "common.h"

#include "cthulhu/hlir/query.h"

#include "report/report.h"

#include "std/vector.h"

#include "base/panic.h"

#include <stdint.h>

static h2_t *decl_open(const node_t *node, const char *name, const h2_t *type, h2_kind_t expected, h2_resolve_config_t resolve)
{
    h2_t *self = h2_decl(eHlir2Resolve, node, type, name);

    self->expected = expected;
    self->sema = resolve.sema;
    self->user = resolve.user;
    self->fnResolve = resolve.fnResolve;

    return self;
}

static void decl_close(h2_t *decl, h2_kind_t kind)
{
    CTASSERTF(h2_is(decl, eHlir2Resolve), "decl `%s` is not a resolve, found %s", h2_get_name(decl), h2_kind_to_string(decl->kind));
    CTASSERTF(decl->expected == kind, "decl %s is an unresolved %s, expected %s", h2_get_name(decl), h2_kind_to_string(decl->expected), h2_kind_to_string(kind));

    decl->kind = kind;
}

static void close_function(h2_t *decl, vector_t *pendingLocals, h2_t *body)
{
    CTASSERTF(decl->expected == eHlir2DeclFunction, "decl %s is not a function", h2_get_name(decl));

    decl_close(decl, eHlir2DeclFunction);
    decl->locals = pendingLocals;
    decl->body = body;
}

h2_t *h2_resolve(h2_cookie_t *cookie, h2_t *decl)
{
    if (!h2_is(decl, eHlir2Resolve)) { return decl; }

    size_t index = vector_find(cookie->stack, decl);
    if (index != SIZE_MAX)
    {
        // TODO: better reporting
        report(cookie->reports, eFatal, decl->node, "cyclic dependency when resolving %s", h2_get_name(decl));
        return h2_error(decl->node, "cyclic dependency");
    }

    vector_push(&cookie->stack, decl);

    decl->fnResolve(cookie, decl->sema, decl, decl->user);

    vector_drop(cookie->stack);

    return decl;
}

h2_t *h2_open_global(const node_t *node, const char *name, const h2_t *type, h2_resolve_config_t resolve)
{
    return decl_open(node, name, type, eHlir2DeclGlobal, resolve);
}

h2_t *h2_open_function(const node_t *node, const char *name, const h2_t *signature, h2_resolve_config_t resolve)
{
    h2_t *self = decl_open(node, name, signature, eHlir2DeclFunction, resolve);
    self->pendingLocals = vector_new(4);
    return self;
}

void h2_close_global(h2_t *self, h2_t *value)
{
    decl_close(self, eHlir2DeclGlobal);
    self->global = value;
}

void h2_close_function(h2_t *self, h2_t *body)
{
    close_function(self, self->pendingLocals, body);
}

h2_t *h2_decl_param(const node_t *node, const char *name, const h2_t *type)
{
    return h2_decl(eHlir2DeclParam, node, type, name);
}

h2_t *h2_decl_local(const node_t *node, const char *name, const h2_t *type)
{
    return h2_decl(eHlir2DeclLocal, node, type, name);
}

static const h2_resolve_config_t kEmptyConfig = {
    .user = NULL,
    .sema = NULL,
    .fnResolve = NULL
};

h2_t *h2_decl_global(const node_t *node, const char *name, const h2_t *type, h2_t *value)
{
    h2_t *self = h2_open_global(node, name, type, kEmptyConfig);
    h2_close_global(self, value);
    return self;
}

h2_t *h2_decl_function(const node_t *node, const char *name, const h2_t *signature, vector_t *locals, h2_t *body)
{
    h2_t *self = decl_open(node, name, signature, eHlir2DeclFunction, kEmptyConfig);
    close_function(self, locals, body);
    return self;
}

void h2_add_local(h2_t *self, h2_t *decl)
{
    CTASSERTF(h2_is(self, eHlir2Resolve), "cannot add locals to a completed declaration %s", h2_to_string(self));
    CTASSERTF(self->expected == eHlir2DeclFunction, "cannot add locals to a non-function declaration %s", h2_to_string(self));

    vector_push(&self->pendingLocals, decl);
}

void h2_set_attrib(h2_t *self, const h2_attrib_t *attrib)
{
    self->attrib = attrib;
}
