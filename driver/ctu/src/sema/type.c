#include "ctu/sema/type.h"
#include "ctu/sema/sema.h"
#include "ctu/ast.h"

#include "cthulhu/hlir/query.h"

#include "std/vector.h"
#include "std/str.h"

#include "report/report.h"

#include "base/panic.h"

///
/// sema type
///

static h2_t *sema_type_name(h2_t *sema, const ctu_t *type)
{
    size_t len = vector_len(type->typeName);
    h2_t *ns = sema;
    for (size_t i = 0; i < len - 1; i++)
    {
        const char *segment = vector_get(type->typeName, i);
        ns = ctu_get_namespace(ns, segment);
        if (ns == NULL)
        {
            report(sema->reports, eFatal, type->node, "namespace `%s` not found", segment);
            return h2_error(type->node, "namespace not found");
        }
    }

    const char *name = vector_tail(type->typeName);
    h2_t *decl = ctu_get_type(ns, name);
    if (decl == NULL)
    {
        report(sema->reports, eFatal, type->node, "type `%s` not found", name);
        return h2_error(type->node, "type not found");
    }

    return decl;
}

static h2_t *ctu_sema_type_pointer(h2_t *sema, const ctu_t *type)
{
    h2_t *pointee = ctu_sema_type(sema, type->pointer);
    return h2_type_pointer(type->node, format("*%s", h2_get_name(pointee)), pointee);
}

h2_t *ctu_sema_type(h2_t *sema, const ctu_t *type)
{
    CTASSERT(type != NULL);

    switch (type->kind)
    {
    case eCtuTypePointer: return ctu_sema_type_pointer(sema, type);
    case eCtuTypeName: return sema_type_name(sema, type);

    default: NEVER("invalid type kind %d", type->kind);
    }
}

///
/// query type
///

bool ctu_type_is(const h2_t *type, h2_kind_t kind)
{
    while (h2_is(type, eHlir2Qualify))
    {
        type = type->qualify;
    }

    return h2_is(type, kind);
}

///
/// format type
///

const char *ctu_type_string(const h2_t *type)
{
    return h2_to_string(type); // TODO: make this match cthulhu type signatures
}
