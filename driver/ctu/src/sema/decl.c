#include "ctu/sema/decl.h"
#include "ctu/sema/type.h"
#include "ctu/sema/expr.h"

#include "cthulhu/tree/query.h"

#include "cthulhu/util/util.h"

#include "std/vector.h"
#include "std/str.h"

#include "base/panic.h"

#include <string.h>

///
/// attributes
///

static const attribs_t kAttribPrivate = {
    .link = eLinkModule,
    .visibility = eVisiblePrivate
};

static const attribs_t kAttribExport = {
    .link = eLinkExport,
    .visibility = eVisiblePublic
};

static const attribs_t kAttribForward = {
    .link = eLinkImport,
    .visibility = eVisiblePublic
};

static const attribs_t kAttribImport = {
    .link = eLinkImport,
    .visibility = eVisiblePrivate
};

///
/// decl resolution
///

static ctu_t *begin_resolve(tree_t *sema, tree_t *self, void *user, ctu_kind_t kind)
{
    ctu_t *decl = user;
    CTASSERTF(decl->kind == kind, "decl %s is not a %d", decl->name, kind);

    util_set_current_module(sema, sema);
    ctu_set_current_symbol(sema, self);

    return decl;
}

static void ctu_resolve_global(cookie_t *cookie, tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclGlobal);

    tree_t *type = decl->type == NULL ? NULL : ctu_sema_type(sema, decl->type);
    tree_t *expr = decl->value == NULL ? NULL : ctu_sema_rvalue(sema, decl->value, type);

    CTASSERT(expr != NULL || type != NULL);

    const tree_t *realType = expr == NULL ? type : tree_get_type(expr);

    // TODO: handle arrays
    tree_storage_t storage = {
        .storage = realType,
        .size = 1,
        .quals = decl->mut ? eQualMutable : eQualConst
    };
    self->type = tree_type_reference(self->node, self->name, realType);
    tree_set_storage(self, storage);
    tree_close_global(self, expr);
}

static void ctu_resolve_function(cookie_t *cookie, tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclFunction);

    size_t len = vector_len(self->params);
    const size_t sizes[eCtuTagTotal] = {
        [eCtuTagValues] = len
    };

    tree_t *ctx = tree_module(sema, decl->node, decl->name, eCtuTagTotal, sizes);

    for (size_t i = 0; i < len; i++)
    {
        tree_t *param = vector_get(self->params, i);
        ctu_add_decl(ctx, eCtuTagValues, param->name, param);
    }

    tree_t *body = decl->body == NULL ? NULL : ctu_sema_stmt(ctx, self, decl->body);
    if (body != NULL)
    {
        const tree_t *ty = tree_fn_get_return(self);
        if (util_types_equal(ty, ctu_get_void_type()))
        {
            vector_push(&body->stmts, tree_stmt_return(self->node, tree_expr_unit(self->node, ty)));
        }
    }
    tree_close_function(self, body);
}

static void ctu_resolve_type(cookie_t *cookie, tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclTypeAlias);
    CTASSERTF(decl->type != NULL, "decl %s has no type", decl->name);

    const tree_t *temp = tree_resolve(tree_get_cookie(sema), ctu_sema_type(sema, decl->typeAlias)); // TODO: doesnt support newtypes, also feels icky
    tree_close_decl(self, temp);
}

static void ctu_resolve_struct(cookie_t *cookie, tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclStruct);

    size_t len = vector_len(decl->fields);
    vector_t *items = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *field = vector_get(decl->fields, i);
        tree_t *type = ctu_sema_type(sema, field->fieldType);
        char *name = field->name == NULL ? format("field%zu", i) : field->name;
        tree_t *item = tree_decl_field(field->node, name, type);

        vector_set(items, i, item);
    }

    tree_close_struct(self, items);
}

/* TODO: set visibility inside forwarding */

///
/// forward declarations
///

static tree_t *ctu_forward_global(tree_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclGlobal, "decl %s is not a global", decl->name);
    CTASSERTF(decl->type != NULL || decl->value != NULL, "decl %s has no type and no init expr", decl->name);

    tree_t *type = decl->type == NULL ? NULL : ctu_sema_type(sema, decl->type);

    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_global
    };

    tree_storage_t storage = {
        .storage = type,
        .size = 1,
        .quals = decl->mut ? eQualMutable : eQualConst
    };

    tree_t *global = tree_open_global(decl->node, decl->name, type, resolve);
    tree_set_storage(global, storage);

    return global;
}

static tree_t *ctu_forward_function(tree_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclFunction, "decl %s is not a function", decl->name);

    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_function
    };

    size_t len = vector_len(decl->params);
    vector_t *params = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *param = vector_get(decl->params, i);
        tree_t *type = ctu_sema_type(sema, param->paramType);
        tree_t *it = tree_decl_param(param->node, param->name, type);
        vector_set(params, i, it);
    }

    tree_t *returnType = decl->returnType == NULL ? ctu_get_void_type() : ctu_sema_type(sema, decl->returnType);
    tree_t *signature = tree_type_closure(decl->node, decl->name, returnType, params, eArityFixed);

    return tree_open_function(decl->node, decl->name, signature, resolve);
}

static tree_t *ctu_forward_type(tree_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclTypeAlias, "decl %s is not a type alias", decl->name);

    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_type
    };

    return tree_open_decl(decl->node, decl->name, resolve);
}

static tree_t *ctu_forward_struct(tree_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclStruct, "decl %s is not a struct", decl->name);

    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_struct
    };

    return tree_open_struct(decl->node, decl->name, resolve);
}

static ctu_forward_t forward_decl_inner(tree_t *sema, ctu_t *decl)
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
    default: NEVER("invalid decl kind %d", decl->kind);
    }
}

ctu_forward_t ctu_forward_decl(tree_t *sema, ctu_t *decl)
{
    ctu_forward_t fwd = forward_decl_inner(sema, decl);

    if (decl->kind == eCtuDeclFunction && decl->body == NULL)
    {
        tree_set_attrib(fwd.decl, decl->exported ? &kAttribForward : &kAttribImport);
    }
    else
    {
        tree_set_attrib(fwd.decl, decl->exported ? &kAttribExport : &kAttribPrivate);
    }

    return fwd;
}
