#include "ctu/sema/decl/function.h"
#include "ctu/sema/decl/resolve.h"

#include "ctu/sema/sema.h"
#include "ctu/sema/type.h"
#include "ctu/sema/expr.h"

#include "cthulhu/util/type.h"

#include "cthulhu/tree/query.h"

#include "std/vector.h"

static void add_param(ctu_sema_t *sema, tree_t *param)
{
    const tree_t *ty = tree_get_type(param);
    const char *id = tree_get_name(param);
    const node_t *node = tree_get_node(param);

    if (!tree_is(ty, eTreeTypeStruct))
    {
        ctu_add_decl(sema->sema, eCtuTagValues, id, param);
        return;
    }

    tree_t *ref = tree_type_reference(node, id, ty);

    tree_storage_t storage = {
        .storage = ty,
        .size = 1,
        .quals = eQualMutable
    };
    tree_t *local = tree_decl_local(node, id, storage, ref);
    tree_add_local(sema->decl, local);
    ctu_add_decl(sema->sema, eCtuTagValues, id, local);

    tree_t *init = tree_stmt_assign(node, local, param);
    vector_push(&sema->block, init);
}

void ctu_resolve_function(tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclFunction);
    tree_t *fn = tree_resolve_type(self);
    if (tree_is(fn, eTreeError)) { return; }

    const node_t *node = tree_get_node(fn);
    vector_t *params = tree_fn_get_params(fn);
    const tree_t *returnType = tree_fn_get_return(fn);

    size_t len = vector_len(params);
    const size_t sizes[eCtuTagTotal] = {
        [eCtuTagValues] = len
    };

    tree_t *ctx = tree_module(sema, decl->node, decl->name, eCtuTagTotal, sizes);
    ctu_sema_t inner = ctu_sema_init(ctx, fn, vector_new(0));

    for (size_t i = 0; i < len; i++)
    {
        tree_t *param = vector_get(params, i);
        add_param(&inner, param);
    }

    ctu_t *fnBody = decl->body;

    if (fnBody == NULL)
    {
        tree_close_function(fn, NULL);
        return;
    }

    tree_t *body = ctu_sema_stmt(&inner, fnBody);
    vector_push(&inner.block, body);

    if (fnBody->kind == eCtuStmtList)
    {
        if (util_types_equal(returnType, ctu_get_void_type()))
        {
            tree_t *ret = tree_stmt_return(node, tree_expr_unit(node, returnType));
            vector_push(&inner.block, ret);
        }
    }

    tree_t *block = tree_stmt_block(node, inner.block);
    tree_close_function(self, block);
}

void ctu_resolve_function_type(tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclFunction);
    ctu_sema_t inner = ctu_sema_init(sema, NULL, vector_new(0));

    size_t len = vector_len(decl->params);
    vector_t *params = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *param = vector_get(decl->params, i);
        tree_t *type = ctu_sema_type(&inner, param->paramType);
        tree_t *it = tree_decl_param(param->node, param->name, type);
        vector_set(params, i, it);
    }

    arity_t arity = (decl->variadic != NULL) ? eArityVariable : eArityFixed;
    tree_t *returnType = decl->returnType == NULL
        ? ctu_get_void_type()
        : ctu_sema_type(&inner, decl->returnType);

    tree_t *signature = tree_type_closure(decl->node, decl->name, returnType, params, arity);

    tree_set_type(self, signature);
    self->params = params;
}