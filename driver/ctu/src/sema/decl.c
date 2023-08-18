#include "ctu/sema/decl.h"
#include "ctu/sema/type.h"
#include "ctu/sema/expr.h"

#include "cthulhu/hlir/query.h"

#include "std/vector.h"
#include "std/str.h"

#include "base/panic.h"

#include <string.h>

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
    h2_t *expr = decl->value == NULL ? NULL : ctu_sema_rvalue(sema, decl->value, type);

    CTASSERT(expr != NULL || type != NULL);

    self->type = type == NULL ? h2_get_type(expr) : type;
    h2_close_global(self, expr);
}

static void ctu_resolve_function(h2_cookie_t *cookie, h2_t *sema, h2_t *self, void *user)
{
    ctu_t *decl = user;
    CTASSERTF(decl->kind == eCtuDeclFunction, "decl %s is not a function", decl->name);

    h2_t *body = ctu_sema_stmt(sema, self, decl->body);
    h2_close_function(self, body);
}

static void ctu_resolve_type(h2_cookie_t *cookie, h2_t *sema, h2_t *self, void *user)
{
    ctu_t *decl = user;
    CTASSERTF(decl->kind == eCtuDeclTypeAlias, "decl %s is not a type alias", decl->name);
    CTASSERTF(decl->type != NULL, "decl %s has no type", decl->name);

    h2_t *temp = h2_resolve(h2_get_cookie(sema), ctu_sema_type(sema, decl->typeAlias)); // TODO: doesnt support newtypes, also feels icky
    h2_close_decl(self, temp);
}

static void ctu_resolve_struct(h2_cookie_t *cookie, h2_t *sema, h2_t *self, void *user)
{
    ctu_t *decl = user;
    CTASSERTF(decl->kind == eCtuDeclStruct, "decl %s is not a struct", decl->name);

    size_t len = vector_len(decl->fields);
    vector_t *items = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *field = vector_get(decl->fields, i);
        h2_t *type = ctu_sema_type(sema, field->fieldType);
        char *name = field->name == NULL ? format("field%zu", i) : field->name;
        h2_t *item = h2_decl_field(field->node, name, type);

        vector_set(items, i, item);
    }

    h2_close_struct(self, items);
}

/* TODO: set visibility inside forwarding */

///
/// forward declarations
///

static h2_t *ctu_forward_global(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclGlobal, "decl %s is not a global", decl->name);
    CTASSERTF(decl->type != NULL || decl->value != NULL, "decl %s has no type and no init expr", decl->name);

    h2_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_global
    };

    h2_t *type = decl->type == NULL ? NULL : ctu_sema_type(sema, decl->type);
    h2_t *global = h2_open_global(decl->node, decl->name, type, resolve);

    return global;
}

static h2_t *ctu_forward_function(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclFunction, "decl %s is not a function", decl->name);

    h2_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_function
    };

    h2_t *returnType = ctu_sema_type(sema, decl->returnType);
    h2_t *signature = h2_type_closure(decl->node, decl->name, returnType, vector_of(0), eArityFixed);

    return h2_open_function(decl->node, decl->name, signature, resolve);
}

static h2_t *ctu_forward_type(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclTypeAlias, "decl %s is not a type alias", decl->name);

    h2_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_type
    };

    return h2_open_decl(decl->node, decl->name, resolve);
}

static h2_t *ctu_forward_struct(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclStruct, "decl %s is not a struct", decl->name);

    h2_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_struct
    };

    return h2_open_struct(decl->node, decl->name, resolve);
}

static ctu_forward_t forward_decl_inner(h2_t *sema, ctu_t *decl)
{
    switch (decl->kind)
    {
    case eCtuDeclGlobal: {
        ctu_forward_t fwd = {
            .tag = eCtuTagValues,
            .decl = ctu_forward_global(sema, decl),
        };
        return fwd;
    }
    case eCtuDeclFunction: {
        ctu_forward_t fwd = {
            .tag = eCtuTagFunctions,
            .decl = ctu_forward_function(sema, decl),
        };
        return fwd;
    }
    case eCtuDeclTypeAlias: {
        ctu_forward_t fwd = {
            .tag = eCtuTagTypes,
            .decl = ctu_forward_type(sema, decl)
        };
        return fwd;
    }
    case eCtuDeclStruct: {
        ctu_forward_t fwd = {
            .tag = eCtuTagTypes,
            .decl = ctu_forward_struct(sema, decl)
        };
        return fwd;
    }
    default:
        NEVER("invalid decl kind %d", decl->kind);
    }
}

ctu_forward_t ctu_forward_decl(h2_t *sema, ctu_t *decl)
{
    ctu_forward_t fwd = forward_decl_inner(sema, decl);

    h2_set_attrib(fwd.decl, decl->exported ? &kAttribExport : &kAttribPrivate);

    return fwd;
}
