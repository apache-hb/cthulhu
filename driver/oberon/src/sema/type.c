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

h2_t *obr_sema_type(h2_t *sema, obr_t *type)
{
    switch (type->kind)
    {
    case eObrTypeName: return sema_type_name(sema, type);
    case eObrTypeQual: return sema_type_qual(sema, type);

    default: NEVER("unknown type kind %d", type->kind);
    }
}
