#include "ctu/sema/decl.h"
#include "ctu/sema/type.h"
#include "ctu/sema/expr.h"

#include "cthulhu/hlir/query.h"

#include "std/vector.h"

#include "base/panic.h"

///
/// attributes
///

static const h2_attrib_t kAttribPrivate = {
    .link = eLinkModule,
    .visibility = eVisiblePrivate
};

static const h2_attrib_t kAttribExport = {
    .link = eLinkExport,
    .visibility = eVisiblePublic
};

///
/// decl resolution
///

static void ctu_resolve_global(h2_cookie_t *cookie, h2_t *sema, h2_t *self, void *user)
{
    ctu_t *decl = user;
    CTASSERTF(decl->kind == eCtuDeclGlobal, "decl %s is not a global", decl->name);

    h2_t *type = decl->type == NULL ? NULL : ctu_sema_type(sema, decl->type);
    h2_t *expr = decl->global == NULL ? NULL : ctu_sema_rvalue(sema, decl->global, type);

    CTASSERT(expr != NULL || type != NULL);

    self->type = type == NULL ? h2_get_type(expr) : type;
    h2_close_global(self, expr);
}

static void ctu_resolve_function(h2_cookie_t *cookie, h2_t *sema, h2_t *self, void *user)
{
    ctu_t *decl = user;
    CTASSERTF(decl->kind == eCtuDeclFunction, "decl %s is not a function", decl->name);

    h2_t *body = h2_stmt_block(decl->node, vector_of(0));
    h2_close_function(self, body);
}

static void ctu_resolve_typealias(h2_cookie_t *cookie, h2_t *sema, h2_t *self, void *user)
{
    ctu_t *decl = user;
    CTASSERTF(decl->kind == eCtuDeclTypeAlias, "decl %s is not a type alias", decl->name);
    CTASSERTF(decl->type != NULL, "decl %s has no type", decl->name);

    NEVER("unimplemented");
}

static void ctu_resolve_struct(h2_cookie_t *cookie, h2_t *sema, h2_t *self, void *user)
{
    ctu_t *decl = user;
    CTASSERTF(decl->kind == eCtuDeclStruct, "decl %s is not a struct", decl->name);

    NEVER("unimplemented");
}

///
/// forward declarations
///

static h2_t *ctu_forward_global(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclGlobal, "decl %s is not a global", decl->name);
    CTASSERTF(decl->type != NULL || decl->global != NULL, "decl %s has no type and no init expr", decl->name);

    const h2_attrib_t *attrib = decl->exported ? &kAttribExport : &kAttribPrivate;

    h2_resolve_config_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_global
    };

    h2_t *type = decl->type == NULL ? NULL : ctu_sema_type(sema, decl->type);
    h2_t *global = h2_open_global(decl->node, decl->name, type, resolve);
    h2_set_attrib(global, attrib);

    return global;
}

static h2_t *ctu_forward_function(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclFunction, "decl %s is not a function", decl->name);

    h2_resolve_config_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_function
    };

    h2_t *returnType = ctu_sema_type(sema, decl->returnType);
    h2_t *signature = h2_type_closure(decl->node, decl->name, returnType, vector_of(0), eArityFixed);

    h2_t *function = h2_open_function(decl->node, decl->name, signature, resolve);

    return function;
}

static h2_t *ctu_forward_typealias(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclTypeAlias, "decl %s is not a type alias", decl->name);

    h2_resolve_config_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_typealias
    };

    return h2_error(decl->node, "unimplemented");
}

static h2_t *ctu_forward_struct(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclStruct, "decl %s is not a struct", decl->name);

    h2_resolve_config_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_struct
    };

    return h2_error(decl->node, "unimplemented");
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
    case eCtuDeclTypeAlias: {
        ctu_forward_t fwd = {
            .tag = eTagTypes,
            .decl = ctu_forward_typealias(sema, decl)
        };
        return fwd;
    }
    case eCtuDeclStruct: {
        ctu_forward_t fwd = {
            .tag = eTagTypes,
            .decl = ctu_forward_struct(sema, decl)
        };
        return fwd;
    }

    default:
        NEVER("invalid decl kind %d", decl->kind);
    }
}
