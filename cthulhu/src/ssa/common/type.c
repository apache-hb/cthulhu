#include "common.h"

#include "cthulhu/hlir/query.h"

#include "std/typevec.h"
#include "std/vector.h"

#include "base/memory.h"
#include "base/panic.h"

ssa_type_t *ssa_type_new(ssa_kind_t kind, const char *name)
{
    ssa_type_t *type = ctu_malloc(sizeof(ssa_type_t));
    type->kind = kind;
    type->name = name;
    return type;
}

ssa_type_t *ssa_type_empty(const char *name)
{
    return ssa_type_new(eTypeEmpty, name);
}

ssa_type_t *ssa_type_unit(const char *name)
{
    return ssa_type_new(eTypeUnit, name);
}

ssa_type_t *ssa_type_bool(const char *name)
{
    return ssa_type_new(eTypeBool, name);
}

ssa_type_t *ssa_type_digit(const char *name, sign_t sign, digit_t digit)
{
    ssa_type_digit_t it = { .sign = sign, .digit = digit };
    ssa_type_t *type = ssa_type_new(eTypeDigit, name);
    type->digit = it;
    return type;
}

ssa_type_t *ssa_type_string(const char *name)
{
    return ssa_type_new(eTypeString, name);
}

ssa_type_t *ssa_type_qualify(const char *name, quals_t quals, ssa_type_t *type)
{
    ssa_type_qualify_t it = { .quals = quals, .type = type };
    ssa_type_t *qual = ssa_type_new(eTypeQualify, name);
    qual->qualify = it;
    return qual;
}

ssa_type_t *ssa_type_closure(const char *name, ssa_type_t *result, typevec_t *params)
{
    ssa_type_closure_t it = { .result = result, .params = params };
    ssa_type_t *closure = ssa_type_new(eTypeClosure, name);
    closure->closure = it;
    return closure;
}

static typevec_t *collect_params(vector_t *vector)
{
    size_t len = vector_len(vector);
    typevec_t *result = typevec_of(sizeof(ssa_param_t), len);

    for (size_t i = 0; i < len; i++)
    {
        const h2_t *param = vector_get(vector, i);
        CTASSERTF(h2_is(param, eHlir2DeclParam), "expected param, got %s", h2_to_string(param));

        const char *name = h2_get_name(param);
        const h2_t *type = h2_get_type(param);

        ssa_param_t entry = {
            .name = name,
            .type = ssa_type_from(type)
        };

        typevec_set(result, i, &entry);
    }

    return result;
}

ssa_type_t *ssa_type_from(const h2_t *type)
{
    h2_kind_t kind = h2_get_kind(type);
    const char *name = h2_get_name(type);
    switch (kind)
    {
    case eHlir2TypeEmpty: return ssa_type_empty(name);
    case eHlir2TypeUnit: return ssa_type_unit(name);
    case eHlir2TypeBool: return ssa_type_bool(name);
    case eHlir2TypeDigit: return ssa_type_digit(name, type->sign, type->digit);
    case eHlir2TypeString: return ssa_type_string(name);
    case eHlir2TypeClosure:
        return ssa_type_closure(
            /* name = */ name,
            /* result = */ ssa_type_from(type->result),
            /* params = */ collect_params(type->params)
        );
    case eHlir2Qualify: return ssa_type_qualify(name, type->quals, ssa_type_from(type->qualify));

    default:
        NEVER("unexpected type kind: %s", h2_to_string(type));
    }
}
