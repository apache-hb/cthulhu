#include "repr.h"

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"
#include "report/report.h"
#include "std/str.h"

static const char *repr_pointer(reports_t *reports, const hlir_t *type, bool detail)
{
    const char *inner = ctu_repr(reports, type->ptr, false);
    const char *name = get_hlir_name(type);

    const char *ptr = type->indexable ? "[*]" : "*";

    if (detail && name)
    {
        return format("%s%s (aka '%s')", ptr, inner, name);
    }

    return format("%s%s", ptr, inner);
}

static const char *repr_array(reports_t *reports, const hlir_t *type, bool detail)
{
    const char *inner = ctu_repr(reports, type->element, false);
    const char *name = get_hlir_name(type);

    if (detail)
    {
        return format("[]%s (aka '%s')", inner, name);
    }

    return format("[]%s", inner);
}

static const char *repr_alias(reports_t *reports, const hlir_t *type, bool detail)
{
    const char *inner = ctu_repr(reports, type->alias, false);
    const char *name = get_hlir_name(type);

    if (detail)
    {
        return format("%s (aka %s)", inner, name);
    }

    return inner;
}

const char *ctu_repr(reports_t *reports, const hlir_t *type, bool detail)
{
    const hlir_t *inner = hlir_follow_type(type);
    hlir_kind_t kind = get_hlir_kind(inner);
    switch (kind)
    {
    case eHlirDigit:
    case eHlirBool:
    case eHlirString:
    case eHlirVoid:
        return get_hlir_name(inner);

    case eHlirPointer:
        return repr_pointer(reports, inner, detail);

    case eHlirArray:
        return repr_array(reports, inner, detail);

    case eHlirAlias:
        return repr_alias(reports, inner, detail);

    default:
        ctu_assert(reports, "ctu-repr unexpected %s", hlir_kind_to_string(kind));
        return "unexpected";
    }
}
