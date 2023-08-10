#include "ctu/sema/decl.h"
#include "ctu/sema/type.h"

#include "std/vector.h"

#include "base/panic.h"

static void ctu_resolve_global(h2_cookie_t *cookie, h2_t *self, void *user)
{

}

static void ctu_resolve_function(h2_cookie_t *cookie, h2_t *self, void *user)
{

}

static h2_t *ctu_forward_global(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclGlobal, "decl %s is not a global", decl->name);
    CTASSERTF(decl->type != NULL || decl->global != NULL, "decl %s has no type and no init", decl->name);

    h2_t *type = ctu_sema_type(sema, decl->type);
    h2_t *global = h2_open_global(decl->node, decl->name, type, decl, ctu_resolve_global);

    return global;
}

static h2_t *ctu_forward_function(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclFunction, "decl %s is not a function", decl->name);

    h2_t *returnType = ctu_sema_type(sema, decl->returnType);
    h2_t *signature = h2_type_closure(decl->node, decl->name, returnType, vector_of(0), eArityFixed);

    h2_t *function = h2_open_function(decl->node, decl->name, signature, decl, ctu_resolve_function);

    return function;
}

ctu_forward_t ctu_forward_decl(h2_t *sema, ctu_t *decl)
{
    switch (decl->kind)
    {
    case eCtuDeclGlobal: {
        ctu_forward_t fwd = {
            .tag = eTagValues,
            .decl = ctu_forward_global(sema, decl),
        };
        return fwd;
    }
    case eCtuDeclFunction: {
        ctu_forward_t fwd = {
            .tag = eTagFunctions,
            .decl = ctu_forward_function(sema, decl),
        };
        return fwd;
    }

    default:
        NEVER("invalid decl kind %d", decl->kind);
    }
}
