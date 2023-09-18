#include "ctu/sema/decl.h"
#include "ctu/sema/type.h"
#include "ctu/sema/expr.h"

#include "cthulhu/tree/query.h"

#include "cthulhu/util/util.h"
#include "cthulhu/util/type.h"

#include "report/report.h"

#include "std/vector.h"
#include "std/str.h"

#include "base/panic.h"

///
/// attributes
///

static const tree_attribs_t kAttribPrivate = {
    .link = eLinkModule,
    .visibility = eVisiblePrivate
};

static const tree_attribs_t kAttribExport = {
    .link = eLinkExport,
    .visibility = eVisiblePublic
};

static const tree_attribs_t kAttribForward = {
    .link = eLinkImport,
    .visibility = eVisiblePublic
};

static const tree_attribs_t kAttribImport = {
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
    util_set_current_symbol(sema, self);

    return decl;
}

static void ctu_resolve_global(tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclGlobal);

    ctu_sema_t ctx = ctu_sema_init(sema, self, vector_new(0));

    tree_t *type = decl->type == NULL ? NULL : ctu_sema_type(ctx, decl->type);
    tree_t *expr = decl->value == NULL ? NULL : ctu_sema_rvalue(ctx, decl->value, type);

    CTASSERT(expr != NULL || type != NULL);

    const tree_t *realType = expr == NULL ? type : tree_get_type(expr);

    size_t size = ctu_resolve_storage_size(realType);
    const tree_t *ty = ctu_resolve_storage_type(realType);

    tree_storage_t storage = {
        .storage = ty,
        .size = size,
        .quals = decl->mut ? eQualMutable : eQualConst
    };
    self->type = tree_type_reference(self->node, self->name, realType);
    tree_set_storage(self, storage);
    tree_close_global(self, expr);
}

static void ctu_resolve_function(tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclFunction);

    size_t len = vector_len(self->params);
    const size_t sizes[eCtuTagTotal] = {
        [eCtuTagValues] = len
    };

    tree_t *ctx = tree_module(sema, decl->node, decl->name, eCtuTagTotal, sizes);
    ctu_sema_t inner = ctu_sema_init(ctx, self, vector_new(0));

    for (size_t i = 0; i < len; i++)
    {
        tree_t *param = vector_get(self->params, i);
        ctu_add_decl(ctx, eCtuTagValues, param->name, param);
    }

    tree_t *body = decl->body == NULL ? NULL : ctu_sema_stmt(inner, decl->body);
    if (body != NULL && tree_is(body, eTreeStmtBlock))
    {
        const tree_t *ty = tree_fn_get_return(self);
        if (util_types_equal(ty, ctu_get_void_type()))
        {
            vector_push(&body->stmts, tree_stmt_return(self->node, tree_expr_unit(self->node, ty)));
        }
    }

    tree_close_function(self, body);
}

static void ctu_resolve_type(tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclTypeAlias);
    CTASSERTF(decl->type != NULL, "decl %s has no type", decl->name);

    ctu_sema_t inner = ctu_sema_init(sema, self, vector_new(0));

    const tree_t *temp = tree_resolve(tree_get_cookie(sema), ctu_sema_type(inner, decl->typeAlias)); // TODO: doesnt support newtypes, also feels icky
    tree_close_decl(self, temp);
}

static void ctu_resolve_struct(tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclStruct);
    ctu_sema_t inner = ctu_sema_init(sema, self, vector_new(0));

    size_t len = vector_len(decl->fields);
    vector_t *items = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *field = vector_get(decl->fields, i);
        tree_t *type = ctu_sema_type(inner, field->fieldType);
        char *name = field->name == NULL ? format("field%zu", i) : field->name;
        tree_t *item = tree_decl_field(field->node, name, type);

        vector_set(items, i, item);
    }

    tree_close_struct(self, items);
}

static void ctu_resolve_variant(tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclVariant);
    ctu_sema_t inner = ctu_sema_init(sema, self, vector_new(0));

    const tree_t *underlying = decl->underlying != NULL
        ? ctu_sema_type(inner, decl->underlying)
        : ctu_get_int_type(eDigitInt, eSignSigned); // TODO: have an enum type

    if (!tree_is(underlying, eTreeTypeDigit))
    {
        report(sema->reports, eFatal, decl->node, "decl `%s` has non-integer underlying type", decl->name);
        return;
    }

    size_t len = vector_len(decl->cases);

    tree_t *defaultCase = NULL;
    vector_t *result = vector_of(len);

    for (size_t i = 0; i < len; i++)
    {
        ctu_t *it = vector_get(decl->cases, i);
        CTASSERTF(it->kind == eCtuVariantCase, "decl %s is not a variant case", it->name);

        tree_t *val = ctu_sema_rvalue(inner, it->caseValue, underlying);
        tree_t *field = tree_decl_case(it->node, it->name, val);
        vector_set(result, i, field);

        if (it->defaultCase)
        {
            if (defaultCase != NULL)
            {
                message_t *id = report(sema->reports, eFatal, it->node, "decl `%s` has multiple default cases", decl->name);
                report_append(id, defaultCase->node, "previous default case");
            }
            else
            {
                defaultCase = field;
            }
        }
    }

    tree_close_enum(self, underlying, result, defaultCase);
}

///
/// forward declarations
///

static tree_t *ctu_forward_global(tree_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclGlobal, "decl %s is not a global", decl->name);
    CTASSERTF(decl->type != NULL || decl->value != NULL, "decl %s has no type and no init expr", decl->name);

    ctu_sema_t inner = ctu_sema_init(sema, NULL, vector_new(0));
    tree_t *type = decl->type == NULL ? NULL : ctu_sema_type(inner, decl->type);

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
    ctu_sema_t inner = ctu_sema_init(sema, NULL, vector_new(0));

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
        tree_t *type = ctu_sema_type(inner, param->paramType);
        tree_t *it = tree_decl_param(param->node, param->name, type);
        vector_set(params, i, it);
    }

    arity_t arity = (decl->variadic != NULL) ? eArityVariable : eArityFixed;
    tree_t *returnType = decl->returnType == NULL
        ? ctu_get_void_type()
        : ctu_sema_type(inner, decl->returnType);

    tree_t *signature = tree_type_closure(decl->node, decl->name, returnType, params, arity);

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

static tree_t *ctu_forward_variant(tree_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclVariant, "decl %s is not a variant", decl->name);

    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = ctu_resolve_variant
    };

    return tree_open_enum(decl->node, decl->name, resolve);
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
    case eCtuDeclVariant: {
        ctu_forward_t fwd = {
            .tag = eCtuTagTypes,
            .decl = ctu_forward_variant(sema, decl)
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

    ctu_apply_attribs(sema, fwd.decl, decl->attribs);

    return fwd;
}
