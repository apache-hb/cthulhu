#include "oberon/sema/decl.h"
#include "core/macros.h"
#include "notify/notify.h"
#include "oberon/sema/type.h"
#include "oberon/sema/expr.h"

#include "cthulhu/util/util.h"

#include "base/panic.h"
#include "memory/memory.h"

static visibility_t remap_visibility(obr_visibility_t vis)
{
    switch (vis)
    {
    case eObrVisPrivate: return eVisiblePrivate;
    case eObrVisPublic:
    case eObrVisPublicReadOnly: return eVisiblePublic;
    default: NEVER("remap-visibility %d", vis);
    }
}

static tree_link_t remap_linkage(obr_visibility_t vis)
{
    switch (vis)
    {
    case eObrVisPublic:
    case eObrVisPublicReadOnly:
        return eLinkExport;
    default: return eLinkModule;
    }
}

static void set_attribs(tree_t *decl, obr_visibility_t vis, tree_link_t linkage)
{
    tree_attribs_t attrib = {
        .link = linkage,
        .visibility = remap_visibility(vis)
    };

    tree_set_attrib(decl, ctu_memdup(&attrib, sizeof(tree_attribs_t)));
}

static obr_t *begin_resolve(tree_t *sema, tree_t *self, void *user, obr_kind_t kind)
{
    obr_t *decl = user;
    CTASSERTF(decl->kind == kind, "decl %s is not a %d", decl->name, kind);

    CTU_UNUSED(sema);
    CTU_UNUSED(self);

    return decl;
}

static void set_const_type(tree_t *self, obr_t *decl, const tree_t *type)
{
    tree_t *ref = tree_type_reference(decl->node, decl->name, type);
    tree_storage_t storage = {
        .storage = type,
        .size = 1,
        .quals = eQualConst
    };

    tree_set_storage(self, storage);
    tree_set_type(self, ref);
}

static void resolve_const(tree_t *sema, tree_t *self, void *user)
{
    obr_t *decl = begin_resolve(sema, self, user, eObrDeclConst);

    tree_t *expr = obr_sema_rvalue(sema, decl->value, NULL);

    set_const_type(self, decl, tree_get_type(expr));
    tree_close_global(self, expr);
}

static void resolve_const_type(tree_t *sema, tree_t *self, void *user)
{
    obr_t *decl = begin_resolve(sema, self, user, eObrDeclConst);

    const tree_t *type = self->initial ? tree_get_type(self->initial) : obr_sema_rvalue(sema, decl->value, NULL);
    set_const_type(self, decl, type);
}

static void resolve_var(tree_t *sema, tree_t *self, void *user)
{
    begin_resolve(sema, self, user, eObrDeclVar);

    tree_t *value = obr_default_value(self->node, tree_get_type(self));

    tree_close_global(self, value);
}

static void resolve_type(tree_t *sema, tree_t *self, void *user)
{
    obr_t *decl = begin_resolve(sema, self, user, eObrDeclType);

    tree_t *type = obr_sema_type(sema, decl->type, decl->name);
    tree_t *alias = tree_alias(tree_resolve(tree_get_cookie(sema), type), decl->name);

    tree_close_decl(self, alias);
}

static void resolve_proc(tree_t *sema, tree_t *self, void *user)
{
    obr_t *decl = begin_resolve(sema, self, user, eObrDeclProcedure);

    vector_t *params = tree_fn_get_params(self);
    size_t param_count = vector_len(params);

    size_t locals = vector_len(decl->locals);
    const size_t sizes[eObrTagTotal] = {
        [eObrTagValues] = locals + param_count,
        [eObrTagTypes] = 0,
        [eObrTagProcs] = 0,
        [eObrTagModules] = 0
    };

    tree_t *ctx = tree_module(sema, decl->node, decl->name, eObrTagTotal, sizes);

    for (size_t i = 0; i < locals; i++)
    {
        obr_t *local = vector_get(decl->locals, i);
        tree_t *type = obr_sema_type(sema, local->type, local->name);
        tree_t *ref = tree_type_reference(local->node, local->name, type);
        tree_storage_t storage = {
            .storage = type,
            .size = 1,
            .quals = eQualConst
        };

        tree_t *local_decl = tree_decl_local(local->node, local->name, storage, ref);
        tree_add_local(self, local_decl);
        obr_add_decl(ctx, eObrTagValues, local->name, local_decl);
    }

    for (size_t i = 0; i < param_count; i++)
    {
        tree_t *param = vector_get(params, i);
        obr_add_decl(ctx, eObrTagValues, param->name, param);
    }

    tree_t *body = obr_sema_stmts(ctx, decl->node, decl->body);

    tree_close_function(self, body);
}

static tree_t *forward_const(tree_t *sema, obr_t *decl)
{
    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fn_resolve = resolve_const,
        .fn_resolve_type = resolve_const_type
    };

    // const declarations never have a type, we infer it from the value
    tree_t *it = tree_open_global(decl->node, decl->name, NULL, resolve);
    return it;
}

static tree_t *forward_var(tree_t *sema, obr_t *decl)
{
    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fn_resolve = resolve_var
    };

    tree_t *type = obr_sema_type(sema, decl->type, decl->name);
    tree_t *ref = tree_type_reference(decl->node, decl->name, type);
    tree_storage_t storage = {
        .storage = type,
        .size = 1,
        .quals = eQualMutable
    };

    tree_t *it = tree_open_global(decl->node, decl->name, ref, resolve);
    tree_set_storage(it, storage);
    return it;
}

static tree_t *forward_type(tree_t *sema, obr_t *decl)
{
    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fn_resolve = resolve_type
    };

    return tree_open_decl(decl->node, decl->name, resolve);
}

static tree_t *forward_proc(tree_t *sema, obr_t *decl)
{
    tree_t *result = decl->result == NULL ? obr_get_void_type() : obr_sema_type(sema, decl->result, decl->name);
    size_t len = vector_len(decl->params);
    vector_t *params = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *param = vector_get(decl->params, i);
        CTASSERTF(param->kind == eObrParam, "param %s is not a param (type=%d)", param->name, param->kind);

        tree_t *type = obr_sema_type(sema, param->type, param->name);
        vector_set(params, i, tree_decl_param(param->node, param->name, type));
    }

    tree_t *signature = tree_type_closure(decl->node, decl->name, result, params, eArityFixed);

    // if this is an extern declaration it doesnt need a body
    if (decl->body == NULL)
    {
        return tree_decl_function(decl->node, decl->name, signature, params, &kEmptyVector, NULL);
    }

    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fn_resolve = resolve_proc
    };

    return tree_open_function(decl->node, decl->name, signature, resolve);
}

static obr_forward_t forward_inner(tree_t *sema, obr_t *decl)
{
    switch (decl->kind)
    {
    case eObrDeclConst: {
        obr_forward_t fwd = {
            .tag = eObrTagValues,
            .decl = forward_const(sema, decl)
        };
        return fwd;
    }

    case eObrDeclVar: {
        obr_forward_t fwd = {
            .tag = eObrTagValues,
            .decl = forward_var(sema, decl)
        };
        return fwd;
    }

    case eObrDeclType: {
        obr_forward_t fwd = {
            .tag = eObrTagTypes,
            .decl = forward_type(sema, decl)
        };
        return fwd;
    }

    default: NEVER("obr-forward-decl %d", decl->kind);
    }
}

obr_forward_t obr_forward_decl(tree_t *sema, obr_t *decl)
{
    if (decl->kind == eObrDeclProcedure)
    {
        tree_t *inner = forward_proc(sema, decl);
        obr_forward_t fwd = {
            .tag = eObrTagProcs,
            .decl = inner
        };

        set_attribs(fwd.decl, decl->visibility, decl->body == NULL ? eLinkImport : eLinkModule);
        return fwd;
    }

    obr_forward_t fwd = forward_inner(sema, decl);
    set_attribs(fwd.decl, decl->visibility, remap_linkage(decl->visibility));
    return fwd;
}

static void obr_resolve_init(tree_t *sema, tree_t *self, void *user)
{
    obr_t *mod = begin_resolve(sema, self, user, eObrModule);

    tree_t *body = obr_sema_stmts(sema, mod->node, mod->init);
    tree_close_function(self, body);
}

static const tree_attribs_t kEntryPoint = {
    .link = eLinkEntryCli
};

tree_t *obr_add_init(tree_t *sema, obr_t *mod)
{
    if (mod->init == NULL) { return NULL; }

    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = mod,
        .fn_resolve = obr_resolve_init
    };

    tree_t *signature = tree_type_closure(mod->node, mod->name, obr_get_void_type(), &kEmptyVector, eArityFixed);
    tree_t *init = tree_open_function(mod->node, mod->name, signature, resolve);
    tree_set_attrib(init, &kEntryPoint);
    return init;
}
