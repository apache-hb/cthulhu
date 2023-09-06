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

    default: NEVER("unknown digit %d", ty.digit);
    }
}

static const char *get_quals(quals_t quals, bool emitConst)
{
    // const is the default
    if (quals & eQualConst) { return emitConst ? "const " : ""; }

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

static const char *format_c89_pointer(c89_emit_t *emit, const char *quals, ssa_type_pointer_t pointer, const char *name)
{
    const char *result = c89_format_type(emit, pointer.pointer, NULL, true);

    return (name == NULL)
        ? format("%s *%s", result, quals)
        : format("%s *%s%s", result, quals, name);
}

const char *c89_format_type(c89_emit_t *emit, const ssa_type_t *type, const char *name, bool emitConst)
{
    CTASSERT(type != NULL);

    const char *quals = get_quals(type->quals, emitConst);

    switch (type->kind)
    {
    case eTypeEmpty: NEVER("cannot emit this type %d", type->kind);
    case eTypeUnit: return (name != NULL) ? format("void %s", name) : "void";

    case eTypeBool: return (name != NULL) ? format("%sbool %s", quals, name) : format("%sbool", quals);
    case eTypeDigit: {
        const char *digitName = get_c89_digit(type->digit);
        return (name != NULL) ? format("%s%s %s", quals, digitName, name) : format("%s%s", quals, digitName);
    }

    case eTypeClosure: return format_c89_closure(emit, quals, type->closure, name);
    case eTypePointer: return format_c89_pointer(emit, quals, type->pointer, name);

    // TODO: emit type definitions for structs
    case eTypeRecord: return (name != NULL) ? format("%sstruct %s %s", quals, type->name, name) : format("%sstruct %s", quals, type->name);

    default: NEVER("unknown type %d", type->kind);
    }
}

const char *c89_format_storage(c89_emit_t *emit, ssa_storage_t storage, const char *name)
{
    const char *inner = c89_format_type(emit, storage.type, NULL, true);
    return format("%s %s[%zu]", inner, name, storage.size);
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
