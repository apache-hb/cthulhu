#include "common.h"

#include "cthulhu/tree/query.h"

#include "std/vector.h"

#include "std/typed/vector.h"

#include "base/memory.h"
#include "base/panic.h"

ssa_type_t *ssa_type_new(ssa_kind_t kind, const char *name, quals_t quals)
{
    ssa_type_t *type = ctu_malloc(sizeof(ssa_type_t));
    type->kind = kind;
    type->quals = quals;
    type->name = name;
    return type;
}

ssa_type_t *ssa_type_empty(const char *name, quals_t quals)
{
    return ssa_type_new(eTypeEmpty, name, quals);
}

ssa_type_t *ssa_type_unit(const char *name, quals_t quals)
{
    return ssa_type_new(eTypeUnit, name, quals);
}

ssa_type_t *ssa_type_bool(const char *name, quals_t quals)
{
    return ssa_type_new(eTypeBool, name, quals);
}

ssa_type_t *ssa_type_digit(const char *name, quals_t quals, sign_t sign, digit_t digit)
{
    ssa_type_digit_t it = { .sign = sign, .digit = digit };
    ssa_type_t *type = ssa_type_new(eTypeDigit, name, quals);
    type->digit = it;
    return type;
}

ssa_type_t *ssa_type_string(const char *name, quals_t quals)
{
    return ssa_type_new(eTypeString, name, quals);
}

ssa_type_t *ssa_type_closure(const char *name, quals_t quals, ssa_type_t *result, typevec_t *params, bool variadic)
{
    ssa_type_closure_t it = {
        .result = result,
        .params = params,
        .variadic = variadic
    };

    ssa_type_t *closure = ssa_type_new(eTypeClosure, name, quals);
    closure->closure = it;
    return closure;
}

static typevec_t *collect_params(vector_t *vector)
{
    size_t len = vector_len(vector);
    typevec_t *result = typevec_of(sizeof(ssa_param_t), len);

    for (size_t i = 0; i < len; i++)
    {
        const tree_t *param = vector_get(vector, i);
        CTASSERTF(tree_is(param, eTreeDeclParam), "expected param, got %s", tree_to_string(param));

        const char *name = tree_get_name(param);
        const tree_t *type = tree_get_type(param);

        ssa_param_t entry = {
            .name = name,
            .type = ssa_type_from(type)
        };

        typevec_set(result, i, &entry);
    }

    return result;
}

static ssa_type_t *ssa_type_inner(const tree_t *type, quals_t quals)
{
    tree_kind_t kind = tree_get_kind(type);
    switch (kind)
    {
    case eTreeTypeEmpty: return ssa_type_empty(tree_get_name(type), quals);
    case eTreeTypeUnit: return ssa_type_unit(tree_get_name(type), quals);
    case eTreeTypeBool: return ssa_type_bool(tree_get_name(type), quals);
    case eTreeTypeDigit: return ssa_type_digit(tree_get_name(type), quals, type->sign, type->digit);
    case eTreeTypeString: return ssa_type_string(tree_get_name(type), quals);
    case eTreeTypeClosure:
        return ssa_type_closure(
            /* name = */ tree_get_name(type),
            /* quals = */ quals,
            /* result = */ ssa_type_from(type->result),
            /* params = */ collect_params(type->params),
            /* variadic = */ type->arity == eArityVariable
        );
    case eTreeTypeQualify: return ssa_type_inner(type->qualify, type->quals | quals);

    default:
        NEVER("unexpected type kind: %s", tree_to_string(type));
    }
}

ssa_type_t *ssa_type_from(const tree_t *type)
{
    return ssa_type_inner(type, eQualDefault);
}
