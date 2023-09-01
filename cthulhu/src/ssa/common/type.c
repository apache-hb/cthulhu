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

ssa_type_t *ssa_type_pointer(const char *name, quals_t quals, ssa_type_t *pointer)
{
    ssa_type_pointer_t it = {
        .pointer = pointer
    };

    ssa_type_t *ptr = ssa_type_new(eTypePointer, name, quals);
    ptr->pointer = it;
    return ptr;
}

ssa_type_t *ssa_type_array(const char *name, quals_t quals, ssa_type_t *element, size_t length)
{
    ssa_type_array_t it = {
        .element = element,
        .length = length
    };

    ssa_type_t *array = ssa_type_new(eTypeArray, name, quals);
    array->array = it;
    return array;
}

ssa_type_t *ssa_type_storage(const char *name, quals_t quals, ssa_type_t *storage, size_t size)
{
    ssa_type_storage_t it = {
        .type = storage,
        .size = size
    };

    ssa_type_t *stor = ssa_type_new(eTypeStorage, name, quals);
    stor->storage = it;
    return stor;
}

static typevec_t *collect_params(const tree_t *type)
{
    vector_t *vec = tree_fn_get_params(type);

    size_t len = vector_len(vec);
    typevec_t *result = typevec_of(sizeof(ssa_param_t), len);

    for (size_t i = 0; i < len; i++)
    {
        const tree_t *param = vector_get(vec, i);
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

static ssa_type_t *ssa_type_inner(const tree_t *type)
{
    tree_kind_t kind = tree_get_kind(type);
    const char *name = tree_get_name(type);
    quals_t quals = tree_ty_get_quals(type);

    switch (kind)
    {
    case eTreeTypeEmpty: return ssa_type_empty(name, quals);
    case eTreeTypeUnit: return ssa_type_unit(name, quals);
    case eTreeTypeBool: return ssa_type_bool(name, quals);
    case eTreeTypeDigit: return ssa_type_digit(name, quals, type->sign, type->digit);
    case eTreeTypeClosure:
        return ssa_type_closure(
            /* name = */ name,
            /* quals = */ quals,
            /* result = */ ssa_type_from(tree_fn_get_return(type)),
            /* params = */ collect_params(type),
            /* variadic = */ tree_fn_get_arity(type) == eArityVariable
        );
    case eTreeTypePointer: return ssa_type_pointer(name, quals, ssa_type_from(type->pointer));
    case eTreeTypeArray: return ssa_type_array(name, quals, ssa_type_from(type->array), type->length);
    case eTreeTypeStorage: return ssa_type_storage(name, quals, ssa_type_from(tree_get_type(type)), type->size);

    default: NEVER("unexpected type kind: %s", tree_to_string(type));
    }
}

ssa_type_t *ssa_type_from(const tree_t *type)
{
    return ssa_type_inner(type);
}
