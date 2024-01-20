#include "c89.h"

#include "arena/arena.h"
#include "std/map.h"
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
    case eDigitSize: return (ty.sign == eSignSigned) ? "ptrdiff_t" : "size_t" ;
    case eDigitPtr: return (ty.sign == eSignSigned) ? "intptr_t" : "uintptr_t";

    case eDigit8: return (ty.sign == eSignUnsigned) ? "uint8_t" : "int8_t";
    case eDigit16: return (ty.sign == eSignUnsigned) ? "uint16_t" : "int16_t";
    case eDigit32: return (ty.sign == eSignUnsigned) ? "uint32_t" : "int32_t";
    case eDigit64: return (ty.sign == eSignUnsigned) ? "uint64_t" : "int64_t";

    case eDigitFast8: return (ty.sign == eSignUnsigned) ? "uint_fast8_t" : "int_fast8_t";
    case eDigitFast16: return (ty.sign == eSignUnsigned) ? "uint_fast16_t" : "int_fast16_t";
    case eDigitFast32: return (ty.sign == eSignUnsigned) ? "uint_fast32_t" : "int_fast32_t";
    case eDigitFast64: return (ty.sign == eSignUnsigned) ? "uint_fast64_t" : "int_fast64_t";

    case eDigitLeast8: return (ty.sign == eSignUnsigned) ? "uint_least8_t" : "int_least8_t";
    case eDigitLeast16: return (ty.sign == eSignUnsigned) ? "uint_least16_t" : "int_least16_t";
    case eDigitLeast32: return (ty.sign == eSignUnsigned) ? "uint_least32_t" : "int_least32_t";
    case eDigitLeast64: return (ty.sign == eSignUnsigned) ? "uint_least64_t" : "int_least64_t";

    case eDigitFloat: return "float";
    case eDigitDouble: return "double";

    default: NEVER("unknown digit %d", ty.digit);
    }
}

static const char *get_quals(quals_t quals, bool emit_const, arena_t *arena)
{
    if (quals & eQualConst) { return emit_const ? "const " : ""; }

    vector_t *vec = vector_new(3, arena);
    if (quals & eQualAtomic) { vector_push(&vec, "_Atomic"); }
    if (quals & eQualVolatile) { vector_push(&vec, "volatile"); }

    return str_join(" ", vec, arena);
}

static const char *format_c89_closure(c89_emit_t *emit, const char *quals, ssa_type_closure_t type, const char *name)
{
    const char *result = c89_format_type(emit, type.result, NULL, true);
    const char *params = c89_format_params(emit, type.params, type.variadic);

    return (name == NULL)
        ? str_format(emit->arena, "%s (*%s)(%s)", result, quals, params)
        : str_format(emit->arena, "%s (*%s%s)(%s)", result, quals, name, params);
}

static const char *format_c89_pointer(c89_emit_t *emit, ssa_type_pointer_t pointer, const char *name)
{
    const char *tmp = (name == NULL) ? "*" : str_format(emit->arena, "*%s", name);

    return c89_format_type(emit, pointer.pointer, tmp, true);
}

static const char *format_c89_enum(c89_emit_t *emit, const ssa_type_t *type, const char *name, const char *quals, type_format_t flags)
{
    if (flags & eFormatEmitCxx)
    {
        const ssa_module_t *mod = map_get(emit->modmap, type);
        const char *ns = get_namespace(mod, emit->arena);

        return (name != NULL)
            ? str_format(emit->arena, "%s%s::%s %s", quals, ns, type->name, name)
            : str_format(emit->arena, "%s%s::%s", quals, ns, type->name);
    }
    else
    {
        return (name != NULL)
            // TODO: this is a hack for abi stability
            // update define_enum when this is updated
            ? str_format(emit->arena, "%s %s_underlying_t %s", quals, type->name, name)
            : str_format(emit->arena, "%s %s_underlying_t", quals, type->name);
    }
}

static const char *format_c89_struct(c89_emit_t *emit, const ssa_type_t *type, const char *name, const char *quals, type_format_t flags)
{
    if (flags & eFormatEmitCxx)
    {
        const ssa_module_t *mod = map_get(emit->modmap, type);
        const char *ns = get_namespace(mod, emit->arena);

        return (name != NULL)
            ? str_format(emit->arena, "%s%s::%s %s", quals, ns, type->name, name)
            : str_format(emit->arena, "%s%s::%s", quals, ns, type->name);
    }
    else
    {
        return (name != NULL)
            ? str_format(emit->arena, "%sstruct %s %s", quals, type->name, name)
            : str_format(emit->arena, "%sstruct %s", quals, type->name);
    }
}

const char *c89_format_type(c89_emit_t *emit, const ssa_type_t *type, const char *name, type_format_t flags)
{
    CTASSERT(type != NULL);

    const char *quals = get_quals(type->quals, flags & eFormatEmitConst, emit->arena);

    switch (type->kind)
    {
    case eTypeUnit: return (name != NULL)
        ? str_format(emit->arena, "void %s", name)
        : "void";

    case eTypeBool: return (name != NULL)
        ? str_format(emit->arena, "%sbool %s", quals, name)
        : str_format(emit->arena, "%sbool", quals);

    case eTypeDigit: {
        const char *digit_name = get_c89_digit(type->digit);
        return (name != NULL)
            ? str_format(emit->arena, "%s%s %s", quals, digit_name, name)
            : str_format(emit->arena, "%s%s", quals, digit_name);
    }

    case eTypeOpaque: return (name != NULL)
        ? str_format(emit->arena, "%svoid *%s", quals, name)
        : str_format(emit->arena, "%svoid *", quals);

    case eTypeClosure: return format_c89_closure(emit, quals, type->closure, name);
    case eTypePointer: return format_c89_pointer(emit, type->pointer, name);

    case eTypeEnum: return format_c89_enum(emit, type, name, quals, flags);

    case eTypeStruct: return format_c89_struct(emit, type, name, quals, flags);

    case eTypeEmpty: NEVER("cannot emit empty type `%s`", type->name);
    default: NEVER("unknown type %s", type_to_string(type, emit->arena));
    }
}

const char *c89_format_storage(c89_emit_t *emit, ssa_storage_t storage, const char *name)
{
    char *it = str_format(emit->arena, "%s[%zu]", name, storage.size);
    return c89_format_type(emit, storage.type, it, true);
}

const char *c89_format_params(c89_emit_t *emit, typevec_t *params, bool variadic)
{
    CTASSERT(emit != NULL);
    CTASSERT(params != NULL);

    size_t len = typevec_len(params);
    if (len == 0)
    {
        return variadic ? "" : "void";
    }

    vector_t *args = vector_of(len, emit->arena);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_param_t *param = typevec_offset(params, i);
        const char *it = c89_format_type(emit, param->type, param->name, true);
        vector_set(args, i, (char*)it);
    }

    char *all = str_join(", ", args, emit->arena);
    return variadic ? str_format(emit->arena, "%s, ...", all) : all;
}
