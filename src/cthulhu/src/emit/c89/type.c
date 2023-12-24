#include "c89.h"

#include "cthulhu/util/util.h"

#include "std/vector.h"
#include "std/str.h"

#include "std/typed/vector.h"

#include "base/panic.h"

static const char *get_c89_digit(ssa_type_digit_t ty)
{
    switch (ty.digit)
    {
    case eDigitChar: return (ty.sign == eSignUnsigned) ? "unsigned char" : "char";
    case eDigitShort: return (ty.sign == eSignUnsigned) ? "unsigned short" : "short";
    case eDigitInt: return (ty.sign == eSignUnsigned) ? "unsigned int" : "int";
    case eDigitLong: return (ty.sign == eSignUnsigned) ? "unsigned long" : "long";
    case eDigitSize: return (ty.sign == eSignUnsigned) ? "size_t" : "ptrdiff_t";
    case eDigitPtr: return (ty.sign == eSignUnsigned) ? "uintptr_t" : "intptr_t";

    case eDigit8: return (ty.sign == eSignUnsigned) ? "uint8_t" : "int8_t";
    case eDigit16: return (ty.sign == eSignUnsigned) ? "uint16_t" : "int16_t";
    case eDigit32: return (ty.sign == eSignUnsigned) ? "uint32_t" : "int32_t";
    case eDigit64: return (ty.sign == eSignUnsigned) ? "uint64_t" : "int64_t";

    default: NEVER("unknown digit %d", ty.digit);
    }
}

static const char *get_quals(quals_t quals, bool emit_const)
{
    // const is the default
    if (quals & eQualConst) { return emit_const ? "const " : ""; }

    vector_t *vec = vector_new(3);
    if (quals & eQualAtomic) { vector_push(&vec, "_Atomic"); }
    if (quals & eQualVolatile) { vector_push(&vec, "volatile"); }

    return str_join(" ", vec);
}

static const char *format_c89_closure(c89_emit_t *emit, const char *quals, ssa_type_closure_t type, const char *name)
{
    const char *result = c89_format_type(emit, type.result, NULL, true);
    const char *params = c89_format_params(emit, type.params, type.variadic);

    return (name == NULL)
        ? format("%s (*%s)(%s)", result, quals, params)
        : format("%s (*%s%s)(%s)", result, quals, name, params);
}

static const char *format_c89_pointer(c89_emit_t *emit, ssa_type_pointer_t pointer, const char *name)
{
    const char *tmp = (name == NULL) ? "*" : format("*%s", name);

    return c89_format_type(emit, pointer.pointer, tmp, true);
}

const char *c89_format_type(c89_emit_t *emit, const ssa_type_t *type, const char *name, bool emit_const)
{
    CTASSERT(type != NULL);

    const char *quals = get_quals(type->quals, emit_const);

    switch (type->kind)
    {
    case eTypeUnit: return (name != NULL)
        ? format("void %s", name)
        : "void";

    case eTypeBool: return (name != NULL)
        ? format("%sbool %s", quals, name)
        : format("%sbool", quals);

    case eTypeDigit: {
        const char *digit_name = get_c89_digit(type->digit);
        return (name != NULL)
            ? format("%s%s %s", quals, digit_name, name)
            : format("%s%s", quals, digit_name);
    }

    case eTypeOpaque: return (name != NULL)
        ? format("%svoid *%s", quals, name)
        : format("%svoid *", quals);

    case eTypeClosure: return format_c89_closure(emit, quals, type->closure, name);
    case eTypePointer: return format_c89_pointer(emit, type->pointer, name);

    case eTypeStruct: return (name != NULL)
        ? format("%sstruct %s %s", quals, type->name, name)
        : format("%sstruct %s", quals, type->name);

    case eTypeEmpty: NEVER("cannot emit empty type `%s`", type->name);
    default: NEVER("unknown type %s", type_to_string(type));
    }
}

const char *c89_format_storage(c89_emit_t *emit, ssa_storage_t storage, const char *name)
{
    char *it = format("%s[%zu]", name, storage.size);
    return c89_format_type(emit, storage.type, it, true);
}

const char *c89_format_params(c89_emit_t *emit, typevec_t *params, bool variadic)
{
    size_t len = typevec_len(params);
    if (len == 0)
    {
        return variadic ? "" : "void";
    }

    vector_t *args = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_param_t *param = typevec_offset(params, i);
        const char *it = c89_format_type(emit, param->type, param->name, true);
        vector_set(args, i, (char*)it);
    }

    char *all = str_join(", ", args);
    return variadic ? format("%s, ...", all) : all;
}
