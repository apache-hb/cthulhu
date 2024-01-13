#include "ctu/sema/decl.h"
#include "core/macros.h"
#include "cthulhu/events/events.h"
#include "ctu/driver.h"
#include "ctu/sema/type.h"
#include "ctu/sema/expr.h"

#include "ctu/sema/decl/function.h"

#include "cthulhu/tree/query.h"

#include "cthulhu/util/util.h"
#include "cthulhu/util/type.h"

#include "memory/memory.h"
#include "std/map.h"
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

    CTU_UNUSED(sema);
    CTU_UNUSED(self);

    return decl;
}

static void ctu_resolve_global(tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclGlobal);

    ctu_sema_t ctx = ctu_sema_init(sema, self, vector_new(0));

    tree_t *type = decl->type == NULL ? NULL : ctu_sema_type(&ctx, decl->type);
    tree_t *expr = decl->value == NULL ? NULL : ctu_sema_rvalue(&ctx, decl->value, type);

    CTASSERT(expr != NULL || type != NULL);

    const tree_t *real_type = expr == NULL ? type : tree_get_type(expr);

    size_t size = ctu_resolve_storage_size(real_type);
    const tree_t *ty = ctu_resolve_storage_type(real_type);

    tree_storage_t storage = {
        .storage = ty,
        .size = size,
        .quals = decl->mut ? eQualMutable : eQualConst
    };
    self->type = tree_type_reference(self->node, self->name, real_type);
    tree_set_storage(self, storage);
    tree_close_global(self, expr);
}

static void ctu_resolve_type(tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclTypeAlias);
    CTASSERTF(decl->type_alias != NULL, "decl %s has no type", decl->name);

    ctu_sema_t inner = ctu_sema_init(sema, self, vector_new(0));

    const tree_t *temp = tree_resolve(tree_get_cookie(sema), ctu_sema_type(&inner, decl->type_alias)); // TODO: doesnt support newtypes, also feels icky
    tree_close_decl(self, temp);
}

static vector_t *ctu_collect_fields(tree_t *sema, tree_t *self, ctu_t *decl)
{
    ctu_sema_t inner = ctu_sema_init(sema, self, vector_new(0));
    size_t len = vector_len(decl->fields);

    arena_t *arena = get_global_arena();
    map_t *fields = map_optimal(len, kTypeInfoString, arena);

    vector_t *items = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *field = vector_get(decl->fields, i);
        tree_t *type = ctu_sema_type(&inner, field->field_type);
        char *name = field->name == NULL ? format("field%zu", i) : field->name;
        tree_t *item = tree_decl_field(field->node, name, type);

        tree_t *prev = map_get(fields, name);
        if (prev != NULL)
        {
            msg_notify(sema->reports, &kEvent_DuplicateField, field->node, "aggregate decl `%s` has duplicate field `%s`", decl->name, name);
        }

        vector_set(items, i, item);
        map_set(fields, name, item);
    }

    return items;
}

static void ctu_resolve_struct(tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclStruct);
    vector_t *items = ctu_collect_fields(sema, self, decl);
    tree_close_struct(self, items);
}

static void ctu_resolve_union(tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclUnion);
    vector_t *items = ctu_collect_fields(sema, self, decl);
    tree_close_union(self, items);
}

static void ctu_resolve_variant(tree_t *sema, tree_t *self, void *user)
{
    ctu_t *decl = begin_resolve(sema, self, user, eCtuDeclVariant);
    ctu_sema_t inner = ctu_sema_init(sema, self, vector_new(0));

    const tree_t *underlying = decl->underlying != NULL
        ? ctu_sema_type(&inner, decl->underlying)
        : ctu_get_int_type(eDigitInt, eSignSigned); // TODO: have an enum type

    if (!tree_is(underlying, eTreeTypeDigit))
    {
        msg_notify(sema->reports, &kEvent_InvalidEnumUnderlyingType, decl->node, "decl `%s` has non-integer underlying type", decl->name);
        return;
    }

    size_t len = vector_len(decl->cases);

    tree_t *default_case = NULL;
    vector_t *result = vector_of(len);

    for (size_t i = 0; i < len; i++)
    {
        ctu_t *it = vector_get(decl->cases, i);
        CTASSERTF(it->kind == eCtuVariantCase, "decl %s is not a variant case", it->name);

        tree_t *val = ctu_sema_rvalue(&inner, it->case_value, underlying);
        tree_t *field = tree_decl_case(it->node, it->name, val);
        vector_set(result, i, field);

        if (it->default_case)
        {
            if (default_case != NULL)
            {
                event_t *id = msg_notify(sema->reports, &kEvent_DuplicateDefaultCases, it->node, "decl `%s` has multiple default cases", decl->name);
                msg_append(id, default_case->node, "previous default case");
            }
            else
            {
                default_case = field;
            }
        }
    }

    tree_close_enum(self, underlying, result, default_case);
}

///
/// forward declarations
///

static tree_t *ctu_forward_global(tree_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclGlobal, "decl %s is not a global", decl->name);
    CTASSERTF(decl->type != NULL || decl->value != NULL, "decl %s has no type and no init expr", decl->name);

    ctu_sema_t inner = ctu_sema_init(sema, NULL, vector_new(0));
    tree_t *type = decl->type == NULL ? NULL : ctu_sema_type(&inner, decl->type);

    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fn_resolve = ctu_resolve_global
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
        .fn_resolve = ctu_resolve_function,
        .fn_resolve_type = ctu_resolve_function_type
    };

    return tree_open_function(decl->node, decl->name, NULL, resolve);
}

static tree_t *ctu_forward_type(tree_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclTypeAlias, "decl %s is not a type alias", decl->name);

    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fn_resolve = ctu_resolve_type
    };

    return tree_open_decl(decl->node, decl->name, resolve);
}

static tree_t *ctu_forward_struct(tree_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclStruct, "decl %s is not a struct", decl->name);

    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fn_resolve = ctu_resolve_struct
    };

    return tree_open_struct(decl->node, decl->name, resolve);
}

static tree_t *ctu_forward_union(tree_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclUnion, "decl %s is not a union", decl->name);

    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fn_resolve = ctu_resolve_union
    };

    return tree_open_union(decl->node, decl->name, resolve);
}

static tree_t *ctu_forward_variant(tree_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclVariant, "decl %s is not a variant", decl->name);

    tree_resolve_info_t resolve = {
        .sema = sema,
        .user = decl,
        .fn_resolve = ctu_resolve_variant
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
    case eCtuDeclUnion: {
        ctu_forward_t fwd = {
            .tag = eCtuTagTypes,
            .decl = ctu_forward_union(sema, decl)
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
