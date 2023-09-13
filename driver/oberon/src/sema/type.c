#include "oberon/sema/type.h"
#include "oberon/sema/decl.h"

#include "cthulhu/util/util.h"

#include "report/report.h"

#include "base/panic.h"

static const size_t kLocalModuleTags[] = { eObrTagModules };
static const size_t kGlobalModuleTags[] = { eObrTagImports };
static const size_t kDeclTags[] = { eObrTagTypes };

static const util_search_t kSearchDecl = {
    .localScopeTags = kLocalModuleTags,
    .localScopeTagsLen = sizeof(kLocalModuleTags) / sizeof(size_t),

    .globalScopeTags = kGlobalModuleTags,
    .globalScopeTagsLen = sizeof(kGlobalModuleTags) / sizeof(size_t),

    .declTags = kDeclTags,
    .declTagsLen = sizeof(kDeclTags) / sizeof(size_t)
};

static tree_t *sema_type_name(tree_t *sema, obr_t *type)
{
    tree_t *it = obr_get_type(sema, type->name);
    if (it == NULL)
    {
        return tree_raise(type->node, sema->reports, "type '%s' not found", type->name);
    }

    return it;
}

static tree_t *sema_type_qual(tree_t *sema, obr_t *type)
{
    return util_search_qualified(sema, &kSearchDecl, type->node, type->name, type->symbol);
}

static tree_t *sema_type_pointer(tree_t *sema, obr_t *type, const char *name)
{
    tree_t *it = obr_sema_type(sema, type->pointer, name);
    return tree_type_pointer(type->node, name, it, SIZE_MAX);
}

static tree_t *sema_type_array(tree_t *sema, obr_t *type, const char *name)
{
    tree_t *it = obr_sema_type(sema, type->array, name); // TODO: will the name clash matter?
    return tree_type_array(type->node, name, it, SIZE_MAX); // TODO: compute length
}

static tree_t *sema_type_record(tree_t *sema, obr_t *type, const char *name)
{
    size_t len = vector_len(type->fields);
    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *field = vector_get(type->fields, i);
        tree_t *it = obr_sema_type(sema, field->type, field->name);
        tree_t *decl = tree_decl_field(field->node, field->name, it);
        vector_set(result, i, decl);
    }

    return tree_decl_struct(type->node, name, result);
}

tree_t *obr_sema_type(tree_t *sema, obr_t *type, const char *name)
{
    CTASSERT(type != NULL);

    switch (type->kind)
    {
    case eObrTypeName: return sema_type_name(sema, type);
    case eObrTypeQual: return sema_type_qual(sema, type);
    case eObrTypePointer: return sema_type_pointer(sema, type, name);
    case eObrTypeArray: return sema_type_array(sema, type, name);
    case eObrTypeRecord: return sema_type_record(sema, type, name);

    default: NEVER("unknown type kind %d", type->kind);
    }
}


///
/// query
///

const tree_t *obr_rvalue_type(const tree_t *self)
{
    return tree_ty_load_type(tree_get_type(self));
}
