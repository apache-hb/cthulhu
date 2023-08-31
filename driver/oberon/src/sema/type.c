#include "oberon/sema/type.h"
#include "oberon/sema/decl.h"

#include "report/report.h"

#include "base/panic.h"

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
    tree_t *mod = obr_get_module(sema, type->name);
    if (mod == NULL)
    {
        return tree_raise(type->node, sema->reports, "module '%s' not found", type->name);
    }

    // TODO: dedup with above

    tree_t *it = obr_get_type(mod, type->symbol);
    if (it == NULL)
    {
        return tree_raise(type->node, sema->reports, "type '%s' not found in module '%s'", type->symbol, type->name);
    }

    return it;
}

static tree_t *sema_type_pointer(tree_t *sema, obr_t *type, const char *name)
{
    tree_t *it = obr_sema_type(sema, type->pointer, name);
    return tree_type_pointer(type->node, name, it);
}

static tree_t *sema_type_array(tree_t *sema, obr_t *type, const char *name)
{
    tree_t *it = obr_sema_type(sema, type->array, name); // TODO: will the name clash matter?
    return tree_type_array(type->node, name, it, SIZE_MAX);
}

static tree_t *sema_type_record(tree_t *sema, obr_t *type, const char *name)
{
    size_t len = vector_len(type->fields);
    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *field = vector_get(type->fields, i);
        tree_t *it = obr_sema_type(sema, field->type, field->name);
        vector_set(result, i, it);
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
