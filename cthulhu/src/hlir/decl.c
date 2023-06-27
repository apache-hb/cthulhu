#include "common.h"

#include "std/vector.h"

#include <stdint.h>

h2_t *h2_resolve(h2_cookie_t *cookie, h2_t *decl)
{
    size_t index = vector_find(cookie->stack, decl);
    if (index != SIZE_MAX)
    {
        // TODO: report error
        return NULL;
    }

    vector_push(&cookie->stack, decl);

    decl->fnResolve(cookie, decl, decl->user);

    vector_drop(cookie->stack);

    return decl;
}

h2_t *h2_decl_open(const node_t *node, const char *name, const h2_t *type, void *user, h2_resolve_t fnResolve)
{
    h2_t *self = h2_decl(eHlir2Resolve, node, type, name);
    self->user = user;
    self->fnResolve = fnResolve;
    return self;
}

h2_t *h2_open_global(const node_t *node, const char *name, const h2_t *type)
{
    return h2_decl_open(node, name, type, NULL, NULL);
}

h2_t *h2_open_function(const node_t *node, const char *name, const h2_t *signature)
{
    h2_t *self = h2_decl_open(node, name, signature, NULL, NULL);
    self->locals = vector_new(4);
    return self;
}

void h2_close_global(h2_t *self, h2_t *value)
{
    self->global = value;
}

void h2_close_function(h2_t *self, h2_t *body)
{
    self->body = body;
}

h2_t *h2_decl_param(const node_t *node, const char *name, const h2_t *type)
{
    return h2_decl(eHlir2DeclParam, node, type, name);
}

h2_t *h2_decl_local(const node_t *node, const char *name, const h2_t *type)
{
    return h2_decl(eHlir2DeclLocal, node, type, name);
}

h2_t *h2_decl_global(const node_t *node, const char *name, const h2_t *type, h2_t *value)
{
    h2_t *self = h2_decl(eHlir2DeclGlobal, node, type, name);
    self->global = value;
    return self;
}

h2_t *h2_decl_function(const node_t *node, const char *name, const h2_t *signature, h2_t *body)
{
    h2_t *self = h2_open_function(node, name, signature);
    self->body = body;
    return self;
}

void h2_add_local(h2_t *self, h2_t *decl)
{
    vector_push(&self->locals, decl);
}

void h2_set_attrib(h2_t *self, const h2_attrib_t *attrib)
{
    self->attrib = attrib;
}
