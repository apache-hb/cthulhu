#include "oberon/sema/decl.h"
#include "oberon/sema/type.h"
#include "oberon/sema/expr.h"

#include "report/report.h"

#include "base/util.h"
#include "base/panic.h"

static visibility_t remap_visibility(reports_t *reports, const node_t *node, obr_visibility_t vis)
{
    switch (vis)
    {
    case eObrVisPrivate: return eVisiblePrivate;
    case eObrVisPublic: return eVisiblePublic;
    case eObrVisPublicReadOnly:
        report(reports, eWarn, node, "public read-only is not yet supported");
        return eVisiblePublic;
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

static void set_attribs(tree_t *sema, tree_t *decl, obr_visibility_t vis)
{
    attribs_t attrib = {
        .link = remap_linkage(vis),
        .visibility = remap_visibility(sema->reports, decl->node, vis)
    };

    tree_set_attrib(decl, BOX(attrib));
}

static obr_t *begin_resolve(tree_t *sema, void *user, obr_kind_t kind)
{
    obr_t *decl = user;
    CTASSERTF(decl->kind == kind, "decl %s is not a %d", decl->name, kind);

    obr_set_current_name(sema, decl->name);

    return decl;
}

static void resolve_const(cookie_t *cookie, tree_t *sema, tree_t *self, void *user)
{
    obr_t *decl = begin_resolve(sema, user, eObrDeclConst);

    tree_t *expr = obr_sema_rvalue(sema, decl->value, NULL);
    self->type = tree_type_qualify(self->node, tree_get_type(expr), eQualDefault); ///< TODO: oh no what are you doing?

    tree_close_global(self, expr);
}

static void resolve_var(cookie_t *cookie, tree_t *sema, tree_t *self, void *user)
{
    begin_resolve(sema, user, eObrDeclVar);

    tree_t *value = obr_default_value(self->node, tree_get_type(self));

    tree_close_global(self, value);
}

static void resolve_type(cookie_t *cookie, tree_t *sema, tree_t *self, void *user)
{
    obr_t *decl = begin_resolve(sema, user, eObrDeclType);

    tree_t *type = obr_sema_type(sema, decl->type);
    tree_t *alias = tree_alias(tree_resolve(cookie, type), decl->name);

    tree_close_decl(self, alias);
}

static tree_t *forward_const(tree_t *sema, obr_t *decl)
{
    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = resolve_const
    };

    return tree_open_global(decl->node, decl->name, NULL, resolve);
}

static tree_t *forward_var(tree_t *sema, obr_t *decl)
{
    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = resolve_var
    };

    tree_t *type = obr_sema_type(sema, decl->type);
    tree_t *mut = tree_type_qualify(decl->node, type, eQualMutable);
    return tree_open_global(decl->node, decl->name, mut, resolve);
}

static tree_t *forward_type(tree_t *sema, obr_t *decl)
{
    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = resolve_type
    };

    return tree_open_decl(decl->node, decl->name, resolve);
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
    obr_forward_t fwd = forward_inner(sema, decl);
    set_attribs(sema, fwd.decl, decl->visibility);
    return fwd;
}
