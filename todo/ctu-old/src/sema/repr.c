#include "repr.h"

#include "cthulhu/hlir/h2.h"
#include "cthulhu/hlir/query.h"

#include "report/report.h"
#include "std/str.h"
#include "std/vector.h"

#include <string.h>

static char *repr_tags(quals_t quals)
{
    vector_t *result = vector_new(4);

    if (!(quals & eQualDefault))
    {
        vector_push(&result, (char *)"const");
    }

    if (quals & eQualAtomic)
    {
        vector_push(&result, (char *)"atomic");
    }

    if (quals & eQualVolatile)
    {
        vector_push(&result, (char *)"volatile");
    }

    char *out = str_join(" ", result);

    if (strlen(out) > 0)
    {
        return format("%s ", out);
    }

    return out;
}

static const char *repr_pointer(reports_t *reports, const h2_t *type, bool detail)
{
    const char *inner = ctu_type_repr(reports, type->ptr, false);
    const char *name = h2_get_name(type);

    const char *ptr = type->indexable ? "[*]" : "*";

    if (detail && name)
    {
        return format("%s%s%s (aka '%s')", repr_tags(type), ptr, inner, name);
    }

    return format("%s%s", ptr, inner);
}

static const char *repr_array(reports_t *reports, const h2_t *type, bool detail)
{
    const char *inner = ctu_type_repr(reports, type->element, false);
    const char *name = get_hlir_name(type);

    if (detail)
    {
        if (name != NULL)
        {
            return format("%s[]%s (aka '%s')", repr_tags(type), inner, name);
        }
        else
        {
            return format("%s[]%s", repr_tags(type), inner);
        }
    }

    return format("[]%s", inner);
}

static const char *repr_alias(reports_t *reports, const h2_t *type, bool detail)
{
    const char *inner = ctu_type_repr(reports, type->alias, false);
    const char *name = get_hlir_name(type);

    if (detail)
    {
        return format("%s%s (aka %s)", repr_tags(type), inner, name);
    }

    return inner;
}

static const char *repr_error(const h2_t *type, bool detail)
{
    if (detail)
    {
        return format("invalid type (%s)", type->name);
    }
    else
    {
        return "invalid type";
    }
}

static const char *repr_integral(const h2_t *type, bool detail)
{
    const char *id = h2_get_name(type);
    if (detail)
    {
        return format("%s%s", repr_tags(type), id);
    }

    return id;
}

static const char *repr_closure(reports_t *reports, const h2_t *type, bool detail)
{
    const char *result = ctu_type_repr(reports, type->result, false);
    const char *name = h2_get_name(type);
    if (name == NULL)
    {
        name = "def";
    }

    if (detail)
    {
        size_t totalParams = vector_len(type->params);
        vector_t *paramNames = vector_of(totalParams);
        for (size_t i = 0; i < totalParams; i++)
        {
            const h2_t *param = vector_get(type->params, i);
            const char *paramName = ctu_type_repr(reports, param, false);
            vector_set(paramNames, i, (char *)paramName);
        }

        if (type->arity == eArityVariable)
        {
            vector_push(&paramNames, (char *)"...");
        }

        return format("%s { (%s) -> %s }", name, str_join(", ", paramNames), result);
    }

    return name;
}

const char *ctu_type_repr(reports_t *reports, const h2_t *type, bool detail)
{
    const h2_t *inner = hlir_follow_type(type);
    h2_kind_t kind = get_hlir_kind(inner);
    switch (kind)
    {
    case eHlirDigit:
    case eHlirDecimal:
    case eHlirBool:
    case eHlirString:
    case eHlirEmpty:
    case eHlirUnit:
    case eHlirOpaque:
        return repr_integral(inner, detail);

    case eHlirPointer:
        return repr_pointer(reports, inner, detail);

    case eHlirArray:
        return repr_array(reports, inner, detail);

    case eHlirAlias:
        return repr_alias(reports, inner, detail);

    case eHlirError:
        return repr_error(type, detail);

    case eHlirFunction:
    case eHlirClosure:
        return repr_closure(reports, inner, detail);

    case eHlirStruct:
        return format("struct %s", get_hlir_name(inner));

    case eHlirQualified:
        return ctu_type_repr(reports, get_hlir_type(inner), detail);

    default:
        ctu_assert(reports, "ctu-repr unexpected %s", hlir_kind_to_string(kind));
        return "unexpected";
    }
}

const char *ctu_repr(reports_t *reports, const hlir_t *expr, bool detail)
{
    if (hlir_is(expr, eHlirError))
    {
        return "invalid expression";
    }

    return ctu_type_repr(reports, get_hlir_type(expr), detail);
}
