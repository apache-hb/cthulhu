#include "oberon/sema/decl.h"
#include "oberon/sema/type.h"
#include "oberon/sema/expr.h"

#include "report/report.h"

#include "base/util.h"
#include "base/panic.h"

static h2_visible_t remap_visibility(reports_t *reports, const node_t *node, obr_visibility_t vis)
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

static h2_link_t remap_linkage(obr_visibility_t vis)
{
    switch (vis)
    {
    case eObrVisPublic:
    case eObrVisPublicReadOnly:
        return eLinkExport;
    default: return eLinkModule;
    }
}

static void set_attribs(h2_t *sema, h2_t *decl, obr_visibility_t vis)
{
    h2_attrib_t attrib = {
        .link = remap_linkage(vis),
        .visibility = remap_visibility(sema->reports, decl->node, vis)
    };

    h2_set_attrib(decl, BOX(attrib));
}

static void resolve_const(h2_cookie_t *cookie, h2_t *sema, h2_t *self, void *user)
{
    obr_t *decl = user;
    CTASSERTF(decl->kind == eObrDeclConst, "decl %s is not a const", decl->name);

    h2_t *expr = obr_sema_rvalue(sema, decl->value, NULL);
    self->type = h2_qualify(self->node, h2_get_type(expr), eQualDefault); ///< TODO: oh no what are you doing?

    h2_close_global(self, expr);
}

static void resolve_var(h2_cookie_t *cookie, h2_t *sema, h2_t *self, void *user)
{
    obr_t *decl = user;
    CTASSERTF(decl->kind == eObrDeclVar, "decl %s is not a var", decl->name);

    h2_t *value = obr_default_value(self->node, h2_get_type(self));

    h2_close_global(self, value);
}

static void resolve_type(h2_cookie_t *cookie, h2_t *sema, h2_t *self, void *user)
{
    obr_t *decl = user;
    CTASSERTF(decl->kind == eObrDeclType, "decl %s is not a type", decl->name);

    h2_close_decl(self, obr_sema_type(sema, decl->type));
}

static h2_t *forward_const(h2_t *sema, obr_t *decl)
{
    h2_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = resolve_const
    };

    h2_t *it = h2_open_global(decl->node, decl->name, NULL, resolve);
    set_attribs(sema, it, decl->visibility);

    return it;
}

static h2_t *forward_var(h2_t *sema, obr_t *decl)
{
    h2_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = resolve_var
    };

    h2_t *type = obr_sema_type(sema, decl->type);
    h2_t *mut = h2_qualify(decl->node, type, eQualMutable);
    h2_t *it = h2_open_global(decl->node, decl->name, mut, resolve);
    set_attribs(sema, it, decl->visibility);

    return it;
}

static h2_t *forward_type(h2_t *sema, obr_t *decl)
{
    h2_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fnResolve = resolve_type
    };

    h2_t *it = h2_open_decl(decl->node, decl->name, resolve);
    set_attribs(sema, it, decl->visibility);

    return it;
}

static obr_forward_t forward_inner(h2_t *sema, obr_t *decl)
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

obr_forward_t obr_forward_decl(h2_t *sema, obr_t *decl)
{
    obr_forward_t fwd = forward_inner(sema, decl);
    set_attribs(sema, fwd.decl, decl->visibility);
    return fwd;
}
