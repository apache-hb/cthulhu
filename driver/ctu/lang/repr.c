#include "repr.h"

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"
#include "report/report.h"
#include "std/str.h"

#include <string.h>

static char *repr_tags(const hlir_t *hlir)
{
    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);
    hlir_tags_t tags = attribs->tags;
    vector_t *result = vector_new(4);

    if (tags & eTagConst)
    {
        vector_push(&result, (char *)"const");
    }

    if (tags & eTagAtomic)
    {
        vector_push(&result, (char *)"atomic");
    }

    if (tags & eTagVolatile)
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

static const char *repr_pointer(reports_t *reports, const hlir_t *type, bool detail)
{
    const char *inner = ctu_type_repr(reports, type->ptr, false);
    const char *name = get_hlir_name(type);

    const char *ptr = type->indexable ? "[*]" : "*";

    if (detail && name)
    {
        return format("%s%s%s (aka '%s')", repr_tags(type), ptr, inner, name);
    }

    return format("%s%s", ptr, inner);
}

static const char *repr_array(reports_t *reports, const hlir_t *type, bool detail)
{
    const char *inner = ctu_type_repr(reports, type->element, false);
    const char *name = get_hlir_name(type);

    if (detail)
    {
        return format("%s[]%s (aka '%s')", repr_tags(type), inner, name);
    }

    return format("[]%s", inner);
}

static const char *repr_alias(reports_t *reports, const hlir_t *type, bool detail)
{
    const char *inner = ctu_type_repr(reports, type->alias, false);
    const char *name = get_hlir_name(type);

    if (detail)
    {
        return format("%s%s (aka %s)", repr_tags(type), inner, name);
    }

    return inner;
}

static const char *repr_error(const hlir_t *type, bool detail)
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

static const char *repr_integral(const hlir_t *type, bool detail)
{
    if (detail)
    {
        return format("%s%s", repr_tags(type), get_hlir_name(type));
    }

    return get_hlir_name(type);
}

static const char *repr_closure(reports_t *reports, const hlir_t *type, bool detail)
{
    vector_t *params = closure_params(type);
    const char *result = ctu_type_repr(reports, closure_result(type), false);
    const char *name = get_hlir_name(type);

    if (detail)
    {
        size_t totalParams = vector_len(params);
        vector_t *paramNames = vector_of(totalParams);
        for (size_t i = 0; i < totalParams; i++)
        {
            const char *paramName = ctu_type_repr(reports, vector_get(params, i), false);
            vector_set(paramNames, i, (char *)paramName);
        }

        if (closure_variadic(type))
        {
            vector_push(&paramNames, (char *)"...");
        }

        return format("%s { (%s, ...) -> %s }", name, str_join(", ", paramNames), result);
    }

    return name;
}

const char *ctu_type_repr(reports_t *reports, const hlir_t *type, bool detail)
{
    const hlir_t *inner = hlir_follow_type(type);
    hlir_kind_t kind = get_hlir_kind(inner);
    if (kind == eHlirForward)
    {
        kind = inner->expected;
    }

    switch (kind)
    {
    case eHlirDigit:
    case eHlirBool:
    case eHlirString:
    case eHlirVoid:
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
