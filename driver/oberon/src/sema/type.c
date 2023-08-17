#include "oberon/sema/type.h"
#include "oberon/sema/decl.h"

#include "report/report.h"

#include "base/panic.h"

static h2_t *sema_type_name(h2_t *sema, obr_t *type)
{
    h2_t *it = obr_get_type(sema, type->name);
    if (it == NULL)
    {
        report(sema->reports, eFatal, type->node, "type '%s' not found", type->name);
        return h2_error(type->node, "unresolved type");
    }

    return it;
}

static h2_t *sema_type_qual(h2_t *sema, obr_t *type)
{
    h2_t *mod = obr_get_module(sema, type->name);
    if (mod == NULL)
    {
        report(sema->reports, eFatal, type->node, "module '%s' not found", type->name);
        return h2_error(type->node, "unresolved module");
    }

    // TODO: dedup with above

    h2_t *it = obr_get_type(mod, type->symbol);
    if (it == NULL)
    {
        report(sema->reports, eFatal, type->node, "type '%s' not found in module '%s'", type->symbol, type->name);
        return h2_error(type->node, "unresolved type");
    }

    return it;
}

static h2_t *sema_type_pointer(h2_t *sema, obr_t *type)
{
    h2_t *it = obr_sema_type(sema, type->pointer);
    return h2_type_pointer(type->node, h2_get_name(it), it);
}

static h2_t *sema_type_array(h2_t *sema, obr_t *type)
{
    h2_t *it = obr_sema_type(sema, type->array);
    return h2_type_pointer(type->node, h2_get_name(it), it); // TODO: maybe arrays are not pointers
}

h2_t *obr_sema_type(h2_t *sema, obr_t *type)
{
    CTASSERT(type != NULL);

    switch (type->kind)
    {
    case eObrTypeName: return sema_type_name(sema, type);
    case eObrTypeQual: return sema_type_qual(sema, type);
    case eObrTypePointer: return sema_type_pointer(sema, type);
    case eObrTypeArray: return sema_type_array(sema, type);

    default: NEVER("unknown type kind %d", type->kind);
    }
}
