#include "cthulhu/emit/c89.h"

#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/digit.h"
#include "cthulhu/ssa/ssa.h"

#include "std/vector.h"
#include "std/str.h"

#include "report/report.h"

#include "base/panic.h"

#include "common.h"

#include <stdio.h>

typedef struct c89_ssa_emit_t
{
    emit_t emit;
} c89_ssa_emit_t;

#define REPORTS(emit) ((emit)->emit.reports)

static const char *get_digit_name(c89_ssa_emit_t *emit, digit_t digit, sign_t sign)
{
    switch (digit)
    {
    case eDigitChar:
        return sign == eSigned ? "signed char" : "unsigned char";
    case eDigitShort:
        return sign == eSigned ? "signed short" : "unsigned short";
    case eDigitInt:
        return sign == eSigned ? "signed int" : "unsigned int";
    case eDigitLong:
        return sign == eSigned ? "signed long long" : "unsigned long long";
    case eDigitSize:
        return sign == eSigned ? "ssize_t" : "size_t";
    case eDigitPtr:
        return sign == eSigned ? "intptr_t" : "uintptr_t";
    default:
        report(REPORTS(emit), eInternal, NULL, "unhandled digit: %d", digit);
        return "";
    }
}

static const char *get_type_name(c89_ssa_emit_t *emit, const ssa_type_t *type)
{
    ssa_kind_t kind = type->kind;

    switch (kind)
    {
    case eTypeDigit:
        return get_digit_name(emit, type->digit, type->sign);

    case eTypeUnit:
    case eTypeEmpty:
        return "void";
    case eTypeString:
        return "const char *";

    default:
        report(REPORTS(emit), eInternal, NULL, "unhandled type kind: %d", kind);
        return "";
    }
}

static const char *get_function_name(c89_ssa_emit_t *emit, const ssa_flow_t *function)
{
    switch (function->linkage)
    {
    case eLinkEntryCli:
        return "main";

    default:
        return function->name;
    }
}

static const char *emit_digit(c89_ssa_emit_t *emit, const ssa_type_t *type, const mpz_t digit)
{
    const char *prefix = get_digit_name(emit, type->digit, type->sign);

    char *str = mpz_get_str(NULL, 10, digit);

    return format("((%s)(%s))", prefix, str);
}

static const char *emit_value(c89_ssa_emit_t *emit, const ssa_value_t *value)
{
    ssa_kind_t kind = value->type->kind;

    if (!value->initialized)
    {
        report(REPORTS(emit), eInternal, NULL, "attempting to emit empty value");
        return "";
    }

    switch (kind)
    {
    case eTypeDigit:
        return emit_digit(emit, value->type, value->digit);

    default:
        report(REPORTS(emit), eInternal, NULL, "unhandled value kind: %d", kind);
        return "";
    }
}

static bool value_exists(const ssa_value_t *value)
{
    if (value == NULL) { return false; }
    if (!value->initialized) { return false; }
    if (value->type->kind == eTypeEmpty) { return false; }

    return true;
}

static void c89_emit_ssa_global(c89_ssa_emit_t *emit, const ssa_flow_t *global)
{
    CTASSERT(global->value != NULL);

    const char *name = global->name;
    const char *type = get_type_name(emit, global->type);

    if (value_exists(global->value))
    {
        const char *value = emit_value(emit, global->value);
        WRITE_STRINGF(&emit->emit, "%s %s = %s;\n", type, name, value);
    }
    else
    {
        WRITE_STRINGF(&emit->emit, "%s %s;\n", type, name);
    }
}

static void c89_emit_function(c89_ssa_emit_t *emit, const ssa_flow_t *function)
{
    UNUSED(emit);
    UNUSED(function);
}

static void c89_fwd_function(c89_ssa_emit_t *emit, const ssa_flow_t *function)
{
    CTASSERT(function->type->kind == eTypeSignature);
    const ssa_type_t *type = function->type;

    const char *name = get_function_name(emit, function);
    const char *result = get_type_name(emit, type->result);

    WRITE_STRINGF(&emit->emit, "%s %s(", result, name);
    
    size_t totalParams = vector_len(type->args);
    if (totalParams == 0) 
    {
        WRITE_STRING(&emit->emit, "void");
    }
    else
    {
        for (size_t i = 0; i < totalParams; i++)
        {
            const ssa_type_t *param = vector_get(type->args, i);
            const char *paramType = get_type_name(emit, param);

            WRITE_STRINGF(&emit->emit, "%s", paramType);

            if (i < totalParams - 1)
            {
                WRITE_STRING(&emit->emit, ", ");
            }
        }
    }

    if (type->variadic)
    {
        WRITE_STRING(&emit->emit, ", ...");
    }

    WRITE_STRING(&emit->emit, ");\n");
}

void c89_emit_ssa_modules(reports_t *reports, ssa_module_t *module, io_t *dst)
{
    c89_ssa_emit_t emit = {
        .emit = {
            .reports = reports,
            .io = dst,
            .indent = 0,
        }
    };

    section_t symbols = module->symbols;

    size_t totalGlobals = vector_len(symbols.globals);
    size_t totalFunctions = vector_len(symbols.functions);

    for (size_t i = 0; i < totalGlobals; i++)
    {
        c89_emit_ssa_global(&emit, vector_get(symbols.globals, i));
    }

    for (size_t i = 0; i < totalFunctions; i++)
    {
        c89_fwd_function(&emit, vector_get(symbols.functions, i));
    }

    for (size_t i = 0; i < totalFunctions; i++)
    {
        c89_emit_function(&emit, vector_get(symbols.functions, i));
    }
}
