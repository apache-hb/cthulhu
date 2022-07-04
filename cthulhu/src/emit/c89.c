#include "cthulhu/emit/c89.h"
#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/ops.h"
#include "report/report.h"
#include "std/map.h"

#include "base/util.h"
#include "base/macros.h"
#include "cthulhu/hlir/query.h"
#include "std/set.h"
#include "std/str.h"
#include "std/stream.h"

#include <string.h>

typedef struct
{
    reports_t *reports;
    stream_t *output;

    size_t depth;

    // map of hlir -> string
    map_t *mangledNames;
} c89_emit_t;

static void write_string(c89_emit_t *emit, const char *str)
{
    for (size_t i = 0; i < emit->depth; i++)
    {
        stream_write(emit->output, "  ");
    }

    stream_write(emit->output, str);
}

static void begin_indent(c89_emit_t *emit)
{
    emit->depth++;
}

static void end_indent(c89_emit_t *emit)
{
    emit->depth--;
}

static char *c89_mangle_section(const char *section)
{
    return format("%zu%s", strlen(section), section);
}

static const char *kDigitMangles[eSignTotal][eDigitTotal] = {
    [eSigned] = {[eChar] = "a",
                 [eShort] = "s",
                 [eInt] = "i",
                 [eLong] = "x",

                 [eIntPtr] = "x", // TODO: only right for 64-bit
                 [eIntMax] = "n",
                 [eIntSize] = "x"},
    [eUnsigned] =
        {
            [eChar] = "h",
            [eShort] = "t",
            [eInt] = "j",
            [eLong] = "y",

            [eIntPtr] = "y",
            [eIntMax] = "o",
            [eIntSize] = "y",
        },
};

static const char *c89_mangle_type(c89_emit_t *emit, const hlir_t *type);

static char *get_cv_qualifiers(const hlir_t *type)
{
    const hlir_attributes_t *attribs = get_hlir_attributes(type);

    char quals[3] = "";
    size_t idx = 0;

    if (attribs->tags & eTagVolatile)
    {
        quals[idx++] = 'V';
    }

    if (attribs->tags & eTagConst)
    {
        quals[idx++] = 'K';
    }

    return ctu_strdup(quals);
}

static const char *c89_mangle_pointer(c89_emit_t *emit, const hlir_t *type)
{
    const char *mangled = c89_mangle_type(emit, type->ptr);
    return format("P%s", mangled);
}

static const char *c89_mangle_digit(const hlir_t *type)
{
    digit_t width = type->width;
    sign_t sign = type->sign;

    return kDigitMangles[sign][width];
}

static const char *c89_mangle_qualified(const hlir_t *type)
{
    vector_t *parts = vector_new(4);
    const hlir_t *parent = get_hlir_parent(type);

    while (parent != NULL)
    {
        const char *name = get_hlir_name(parent);
        vector_t *split = str_split(name, ".");
        size_t totalParts = vector_len(parts);

        for (size_t i = 0; i < totalParts; i++)
        {
            char *part = vector_get(split, i);
            char *mangled = c89_mangle_section(part);
            vector_push(&parts, mangled);
        }

        parent = get_hlir_parent(parent);
    }

    char *joinedParts = str_join("", parts);

    const char *name = get_hlir_name(type);

    return format("%s%zu%s", joinedParts, strlen(name), name);
}

static const char *c89_mangle_type_inner(c89_emit_t *emit, const hlir_t *type)
{
    hlir_kind_t kind = get_hlir_kind(type);
    switch (kind)
    {
    case eHlirDigit:
        return c89_mangle_digit(type);

    case eHlirVoid:
        return "v";
    case eHlirBool:
        return "b";

    case eHlirPointer:
        return c89_mangle_pointer(emit, type);

    case eHlirAlias:
    case eHlirStruct:
    case eHlirUnion:
        return c89_mangle_qualified(type);

    case eHlirParam:
        return c89_mangle_type(emit, get_hlir_type(type));

    case eHlirString:
        return "KPc"; // const char *

    default:
        report(emit->reports, eFatal, get_hlir_node(type), "cannot mangle %s", hlir_kind_to_string(kind));
        return "";
    }
}

static const char *c89_mangle_type(c89_emit_t *emit, const hlir_t *type)
{
    const char *mangled = c89_mangle_type_inner(emit, type);
    return format("%s%s", get_cv_qualifiers(type), mangled);
}

static char *c89_mangle_params(c89_emit_t *emit, vector_t *params)
{
    size_t len = vector_len(params);
    vector_t *parts = vector_of(len);

    for (size_t i = 0; i < len; i++)
    {
        hlir_t *param = vector_get(params, i);
        const char *mangled = c89_mangle_type(emit, param);
        vector_set(parts, i, (char *)mangled);
    }

    return str_join("", parts);
}

static char *c89_mangle_decl_name(c89_emit_t *emit, const hlir_t *hlir)
{
    vector_t *parts = vector_new(4);
    const hlir_t *parent = get_hlir_parent(hlir);

    while (parent != NULL)
    {
        const char *name = get_hlir_name(parent);
        vector_t *split = str_split(name, ".");
        size_t totalParts = vector_len(parts);

        for (size_t i = 0; i < totalParts; i++)
        {
            char *part = vector_get(split, i);
            char *mangled = c89_mangle_section(part);
            vector_push(&parts, mangled);
        }

        parent = get_hlir_parent(parent);
    }

    char *joinedParts = str_join("", parts);
    const char *name = get_hlir_name(hlir);
    const char *typesig = "";

    if (hlir_is(hlir, eHlirFunction))
    {
        typesig = c89_mangle_params(emit, closure_params(hlir));
    }

    if (name == NULL)
    {
        name = ""; // TODO: generate a unique id per anonymous field
    }

    return format("_Z%s%zu%s%s", joinedParts, strlen(name), name, typesig);
}

static const char *c89_mangle_name(c89_emit_t *emit, const hlir_t *hlir)
{
    CTASSERTF((hlir->parentDecl == NULL ? hlir_is_imported(hlir) : true), "decl '%s' must have a parent decl", get_hlir_name(hlir));

    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);
    if (attribs->mangle != NULL)
    {
        return attribs->mangle;
    }

    char *result = map_get_ptr(emit->mangledNames, hlir);
    if (result != NULL)
    {
        return result;
    }

    char *mangledName = c89_mangle_decl_name(emit, hlir);
    map_set_ptr(emit->mangledNames, hlir, mangledName);
    return mangledName;
}

#define WRITE_STRINGF(emit, str, ...) write_string(emit, format(str, __VA_ARGS__))
#define WRITE_STRING(emit, str) write_string(emit, str)

static const char *c89_emit_type(c89_emit_t *emit, const hlir_t *hlir, const char *name);
static const char *c89_emit_rvalue(c89_emit_t *emit, const hlir_t *hlir);
static const char *c89_emit_lvalue(c89_emit_t *emit, const hlir_t *hlir);
static void c89_emit_stmt(c89_emit_t *emit, const hlir_t *hlir);

static const char *c89_emit_bool_type(const char *name)
{
    if (name == NULL)
    {
        return "int";
    }
    else
    {
        return format("int %s", name);
    }
}

static const char *c89_emit_void_type(const char *name)
{
    if (name == NULL)
    {
        return "void";
    }
    else
    {
        return format("void %s", name);
    }
}

static const char *kC89SignNames[eSignTotal] = {
    [eUnsigned] = "unsigned",
    [eSigned] = "signed",
};

static const char *kC89DigitNames[eDigitTotal] = {
    [eChar] = "char",
    [eShort] = "short",
    [eInt] = "int",
    [eLong] = "long",
};

static const char *c89_get_digit_symbol(const hlir_t *digit)
{
    sign_t sign = digit->sign;
    digit_t width = digit->width;

    switch (width)
    {
    case eIntSize:
        return sign == eUnsigned ? "size_t" : "ssize_t";
    case eIntPtr:
        return sign == eUnsigned ? "uintptr_t" : "intptr_t";
    case eIntMax:
        return sign == eUnsigned ? "uintmax_t" : "intmax_t";

    default:
        return format("%s %s", kC89SignNames[sign], kC89DigitNames[width]);
    }
}

static const char *c89_emit_digit_type(const hlir_t *digit, const char *name)
{
    const char *symbol = c89_get_digit_symbol(digit);

    if (name != NULL)
    {
        return format("%s %s", symbol, name);
    }
    else
    {
        return symbol;
    }
}

static const char *c89_emit_string_type(const char *name)
{
    if (name == NULL)
    {
        return "const char *";
    }
    else
    {
        return format("const char *%s", name);
    }
}

static const char *c89_emit_pointer_type(c89_emit_t *emit, const hlir_t *hlir, const char *name)
{
    const char *inner = c89_emit_type(emit, hlir->ptr, NULL);

    if (name == NULL)
    {
        return format("%s *", inner);
    }
    else
    {
        return format("%s *%s", inner, name);
    }
}

static const char *c89_emit_array_type(c89_emit_t *emit, const hlir_t *hlir, const char *name)
{
    const char *inner = c89_emit_type(emit, hlir->element, NULL);
    const char *length = c89_emit_rvalue(emit, hlir->length);

    if (name == NULL)
    {
        return format("%s[%s]", inner, length);
    }
    else
    {
        return format("%s %s[%s]", inner, name, length);
    }
}

static const char *c89_emit_aggregate_type(c89_emit_t *emit, const hlir_t *hlir, const char *name,
                                           const char *aggregate)
{
    char *type = format("%s %s", aggregate, c89_mangle_name(emit, hlir));

    if (name == NULL)
    {
        return type;
    }
    else
    {
        return format("%s %s", type, name);
    }
}

static const char *c89_emit_params(c89_emit_t *emit, vector_t *params)
{
    size_t totalParams = vector_len(params);
    if (totalParams == 0)
    {
        return "void";
    }

    vector_t *paramTypes = vector_of(totalParams);

    for (size_t i = 0; i < totalParams; i++)
    {
        const hlir_t *param = vector_get(params, i);
        const char *paramType = c89_emit_type(emit, param, NULL);
        vector_set(paramTypes, i, (char *)paramType);
    }

    return str_join(", ", paramTypes);
}

static const char *c89_emit_closure_type(c89_emit_t *emit, const hlir_t *hlir, const char *name)
{
    const char *result = c89_emit_type(emit, closure_result(hlir), NULL);
    const char *params = c89_emit_params(emit, closure_params(hlir));

    if (name == NULL)
    {
        return format("%s (*)(%s)", result, params);
    }
    else
    {
        return format("%s (*%s)(%s)", result, name, params);
    }
}

static const char *c89_emit_inner_type(c89_emit_t *emit, const hlir_t *hlir, const char *name)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirBool:
        return c89_emit_bool_type(name);
    case eHlirVoid:
        return c89_emit_void_type(name);
    case eHlirDigit:
        return c89_emit_digit_type(hlir, name);
    case eHlirString:
        return c89_emit_string_type(name);
    case eHlirPointer:
        return c89_emit_pointer_type(emit, hlir, name);
    case eHlirArray:
        return c89_emit_array_type(emit, hlir, name);
    case eHlirClosure:
        return c89_emit_closure_type(emit, hlir, name);

    case eHlirAlias:
        return c89_emit_type(emit, hlir->alias, name);

    case eHlirStruct:
        return c89_emit_aggregate_type(emit, hlir, name, "struct");
    case eHlirUnion:
        return c89_emit_aggregate_type(emit, hlir, name, "union");

    case eHlirParam:
        return c89_emit_type(emit, get_hlir_type(hlir), name);

    default:
        ctu_assert(emit->reports, "cannot emit %s as a type", hlir_kind_to_string(kind));
        return "error";
    }
}

static void c89_emit_stmts(c89_emit_t *emit, const hlir_t *hlir)
{
    for (size_t i = 0; i < vector_len(hlir->stmts); i++)
    {
        const hlir_t *stmt = vector_get(hlir->stmts, i);
        c89_emit_stmt(emit, stmt);
    }
}

static void c89_emit_assign(c89_emit_t *emit, const hlir_t *hlir)
{
    const char *dst = c89_emit_lvalue(emit, hlir->dst);
    const char *src = c89_emit_rvalue(emit, hlir->src);

    WRITE_STRINGF(emit, "%s = %s;\n", dst, src);
}

static void c89_emit_branch(c89_emit_t *emit, const hlir_t *hlir)
{
    const char *cond = c89_emit_rvalue(emit, hlir->cond);

    WRITE_STRINGF(emit, "if (%s)\n", cond);
    WRITE_STRING(emit, "{\n");

    begin_indent(emit);
    c89_emit_stmt(emit, hlir->then);
    end_indent(emit);

    WRITE_STRING(emit, "}\n");

    if (hlir->other != NULL)
    {
        WRITE_STRING(emit, "else\n");
        WRITE_STRING(emit, "{\n");

        begin_indent(emit);
        c89_emit_stmt(emit, hlir->other);
        end_indent(emit);

        WRITE_STRING(emit, "}\n");
    }
}

static const char *c89_emit_call(c89_emit_t *emit, const hlir_t *hlir)
{
    const char *func = c89_emit_rvalue(emit, hlir->call);

    size_t totalParams = vector_len(hlir->args);
    vector_t *params = vector_of(totalParams);

    for (size_t i = 0; i < totalParams; i++)
    {
        const hlir_t *param = vector_get(hlir->args, i);
        const char *expr = c89_emit_rvalue(emit, param);
        vector_set(params, i, (void *)expr);
    }

    const char *paramString = str_join(", ", params);

    return format("%s(%s);\n", func, paramString);
}

static void c89_emit_loop(c89_emit_t *emit, const hlir_t *hlir)
{
    const char *cond = c89_emit_rvalue(emit, hlir->cond);

    const node_t node = get_hlir_node(hlir);
    where_t where = get_node_location(node);

    if (hlir->other != NULL)
    {
        WRITE_STRINGF(emit, "int used_loop%ld_%ld = 0;\n", where.firstLine, where.firstColumn);
    }

    WRITE_STRINGF(emit, "while (%s)\n", cond);
    WRITE_STRING(emit, "{\n");

    begin_indent(emit);

    if (hlir->other != NULL)
    {
        WRITE_STRINGF(emit, "used_loop%ld_%ld = 1;\n", where.firstLine, where.firstColumn);
    }

    c89_emit_stmt(emit, hlir->then);
    end_indent(emit);

    WRITE_STRING(emit, "}\n");

    if (hlir->other != NULL)
    {
        WRITE_STRINGF(emit, "if (!used_loop%ld_%ld)\n", where.firstLine, where.firstColumn);
        WRITE_STRING(emit, "{\n");
        begin_indent(emit);
        c89_emit_stmt(emit, hlir->other);
        end_indent(emit);
        WRITE_STRING(emit, "}\n");
    }
}

static void c89_emit_return(c89_emit_t *emit, const hlir_t *hlir)
{
    if (hlir->result != NULL)
    {
        WRITE_STRINGF(emit, "return %s;\n", c89_emit_rvalue(emit, hlir->result));
    }
    else
    {
        WRITE_STRING(emit, "return;\n");
    }
}

static void c89_emit_stmt(c89_emit_t *emit, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirStmts:
        c89_emit_stmts(emit, hlir);
        break;

    case eHlirAssign:
        c89_emit_assign(emit, hlir);
        break;

    case eHlirBranch:
        c89_emit_branch(emit, hlir);
        break;

    case eHlirLoop:
        c89_emit_loop(emit, hlir);
        break;

    case eHlirCall:
        WRITE_STRING(emit, c89_emit_call(emit, hlir));
        break;

    case eHlirReturn:
        c89_emit_return(emit, hlir);
        break;

    default:
        ctu_assert(emit->reports, "cannot emit %s as a statement", hlir_kind_to_string(kind));
        return;
    }
}

static const char *c89_emit_digit_literal(c89_emit_t *emit, const hlir_t *hlir)
{
    const hlir_t *type = get_hlir_type(hlir);
    sign_t sign = type->sign;
    digit_t width = type->width;

    switch (width)
    {
    case eChar:
    case eShort:
    case eInt:
        return format("%s%s", mpz_get_str(NULL, 10, hlir->digit), sign == eUnsigned ? "u" : "");

    case eLong:
        return format("%s%s", mpz_get_str(NULL, 10, hlir->digit), sign == eUnsigned ? "ull" : "ll");

    case eIntMax:
        return format("%s(%s)", sign == eUnsigned ? "UINTMAX_C" : "INTMAX_C", mpz_get_str(NULL, 10, hlir->digit));

    default:
        ctu_assert(emit->reports, "digit literal with (%s, %s) not supported", hlir_sign_to_string(sign),
                   hlir_digit_to_string(width));
        return mpz_get_str(NULL, 10, hlir->digit);
    }
}

static const char *c89_emit_bool_literal(const hlir_t *hlir)
{
    return hlir->boolean ? "1" : "0";
}

static const char *c89_emit_string_literal(const hlir_t *hlir)
{
    char *str = str_normalizen(hlir->string, hlir->stringLength);
    return format("\"%s\"", str);
}

static const char *c89_emit_binary(c89_emit_t *emit, const hlir_t *hlir)
{
    const char *lhs = c89_emit_rvalue(emit, hlir->lhs);
    const char *rhs = c89_emit_rvalue(emit, hlir->rhs);

    const char *op = binary_symbol(hlir->binary);

    return format("(%s %s %s)", lhs, op, rhs);
}

static const char *c89_emit_unary(c89_emit_t *emit, const hlir_t *hlir)
{
    const char *operand = c89_emit_rvalue(emit, hlir->operand);

    if (hlir->unary == eUnaryAbs)
    {
        return format("llabs(%s)", operand);
    }

    const char *op = unary_symbol(hlir->unary);
    return format("(%s %s)", op, operand);
}

static const char *c89_emit_compare(c89_emit_t *emit, const hlir_t *hlir)
{
    const char *lhs = c89_emit_rvalue(emit, hlir->lhs);
    const char *rhs = c89_emit_rvalue(emit, hlir->rhs);

    const char *op = compare_symbol(hlir->compare);
    return format("(%s %s %s)", lhs, op, rhs);
}

static const char *c89_emit_name(c89_emit_t *emit, const hlir_t *hlir)
{
    const char *name = c89_emit_rvalue(emit, hlir->read);
    return format("%s[0]", name);
}

static const char *c89_emit_local_rvalue(const hlir_t *hlir)
{
    return format("%s[0]", hlir->name);
}

static const char *c89_emit_rvalue(c89_emit_t *emit, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirLiteralDigit:
        return c89_emit_digit_literal(emit, hlir);
    case eHlirLiteralBool:
        return c89_emit_bool_literal(hlir);
    case eHlirLiteralString:
        return c89_emit_string_literal(hlir);

    case eHlirName:
        return c89_emit_name(emit, hlir);

    case eHlirUnary:
        return c89_emit_unary(emit, hlir);
    case eHlirBinary:
        return c89_emit_binary(emit, hlir);
    case eHlirCompare:
        return c89_emit_compare(emit, hlir);

    case eHlirGlobal:
        return c89_mangle_name(emit, hlir);

        // TODO: these should be managled to not clash with globals
    case eHlirLocal:
        return c89_emit_local_rvalue(hlir);
    case eHlirParam:
        return get_hlir_name(hlir);

    case eHlirFunction:
        return c89_mangle_name(emit, hlir);

    case eHlirCall:
        return c89_emit_call(emit, hlir);

    default:
        ctu_assert(emit->reports, "cannot emit rvalue for %s", hlir_kind_to_string(kind));
        return "error";
    }
}

static const char *c89_emit_name_lvalue(const hlir_t *hlir)
{
    return format("%s[0]", get_hlir_name(hlir));
}

static const char *c89_emit_lvalue(c89_emit_t *emit, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirName:
    case eHlirLocal:
        return c89_emit_name_lvalue(hlir);

    case eHlirGlobal:
        return c89_mangle_name(emit, hlir);

        // TODO: these should be managled to not clash with globals
    case eHlirParam:
        return get_hlir_name(hlir);

    default:
        ctu_assert(emit->reports, "cannot emit lvalue for %s", hlir_kind_to_string(kind));
        return "error";
    }
}

static const char *c89_emit_type(c89_emit_t *emit, const hlir_t *hlir, const char *name)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    const char *inner = c89_emit_inner_type(emit, hlir, name);
    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);

    if (attribs->tags & eTagConst && kind != eHlirString)
    {
        inner = format("const %s", inner);
    }

    if (attribs->tags & eTagAtomic)
    {
        node_t node = get_hlir_node(hlir);
        report(emit->reports, eWarn, node, "atomic types are not supported yet");
    }

    if (attribs->tags & eTagVolatile)
    {
        inner = format("volatile %s", inner);
    }

    return inner;
}

static void c89_emit_aggregate_decl(c89_emit_t *emit, const hlir_t *hlir, const char *aggregate)
{
    size_t totalFields = vector_len(hlir->fields);

    WRITE_STRINGF(emit, "%s %s\n{\n", aggregate, c89_mangle_name(emit, hlir));

    for (size_t i = 0; i < totalFields; i++)
    {
        const hlir_t *field = vector_get(hlir->fields, i);
        const char *name = c89_mangle_name(emit, field);
        const char *entry = c89_emit_type(emit, get_hlir_type(field), name);

        WRITE_STRINGF(emit, "  %s;\n", entry);
    }

    WRITE_STRING(emit, "};\n");
}

static void c89_emit_typedef(c89_emit_t *emit, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirStruct:
        c89_emit_aggregate_decl(emit, hlir, "struct");
        break;

    case eHlirUnion:
        c89_emit_aggregate_decl(emit, hlir, "union");
        break;

    default:
        ctu_assert(emit->reports, "cannot emit typedef for %s", hlir_kind_to_string(kind));
        break;
    }
}

static void visit_type(c89_emit_t *emit, vector_t **result, const hlir_t *hlir)
{
    if (vector_find(*result, hlir) != SIZE_MAX)
    {
        return;
    }

    const hlir_t *res;
    vector_t *params;
    vector_t *fields;

    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirVoid:
    case eHlirDigit:
    case eHlirBool:
    case eHlirString:
    case eHlirPointer:
    case eHlirArray:
        break;

    case eHlirAlias:
        visit_type(emit, result, hlir->alias);
        break;

    case eHlirField:
        visit_type(emit, result, get_hlir_type(hlir));
        break;

    case eHlirClosure:
        res = closure_result(hlir);
        params = closure_params(hlir);
        visit_type(emit, result, res);
        for (size_t i = 0; i < vector_len(params); i++)
        {
            const hlir_t *param = vector_get(params, i);
            visit_type(emit, result, param);
        }
        break;

    case eHlirStruct:
    case eHlirUnion:
        fields = hlir->fields;
        for (size_t i = 0; i < vector_len(fields); i++)
        {
            const hlir_t *field = vector_get(fields, i);
            visit_type(emit, result, get_hlir_type(field));
        }
        vector_push(result, (void *)hlir);
        break;

    default:
        ctu_assert(emit->reports, "invalid type kind %s", hlir_kind_to_string(kind));
        break;
    }
}

static bool set_check_ptr(set_t *set, const hlir_t *hlir)
{
    if (set_contains_ptr(set, hlir))
    {
        return true;
    }

    set_add_ptr(set, hlir);
    return false;
}

static vector_t *sort_types(c89_emit_t *emit, vector_t *modules)
{
    size_t length = vector_len(modules);
    vector_t *vector = vector_of(length);
    for (size_t i = 0; i < length; i++)
    {
        hlir_t *mod = vector_get(modules, i);
        vector_set(vector, i, mod->types);
    }

    vector_t *allTypes = vector_join(vector);
    size_t totalTypes = vector_len(allTypes);

    vector_t *result = vector_new(totalTypes);

    set_t *uniqueTypes = set_new(totalTypes);

    for (size_t i = 0; i < totalTypes; i++)
    {
        hlir_t *type = vector_get(allTypes, i);
        if (set_check_ptr(uniqueTypes, type))
        {
            continue;
        }

        visit_type(emit, &result, type);
    }

    return result;
}

static void c89_emit_types(c89_emit_t *emit, vector_t *modules)
{
    // now we sort all the types
    vector_t *sortedTypes = sort_types(emit, modules);
    size_t totalSortedTypes = vector_len(sortedTypes);

    for (size_t i = 0; i < totalSortedTypes; i++)
    {
        const hlir_t *type = vector_get(sortedTypes, i);

        c89_emit_typedef(emit, type);
    }
}

static const char *c89_get_linkage(hlir_linkage_t linkage)
{
    switch (linkage)
    {
    case eLinkImported: return "extern ";
    case eLinkInternal: return "static ";
    case eLinkExported: return "";
    default:
        CTASSERT(false, "unknown linkage");
        return "";
    }
}

static void c89_forward_global(c89_emit_t *emit, const hlir_t *hlir)
{
    const char *name = c89_mangle_name(emit, hlir);
    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);
    const char *linkage = c89_get_linkage(attribs->linkage);
    const char *type = c89_emit_type(emit, get_hlir_type(hlir), name);

    WRITE_STRINGF(emit, "%s%s[1];\n", linkage, type);
}

static void c89_emit_global(c89_emit_t *emit, const hlir_t *hlir)
{
    if (hlir_is_imported(hlir))
    {
        return;
    }

    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);
    const char *name = c89_mangle_name(emit, hlir);
    const char *type = c89_emit_type(emit, get_hlir_type(hlir), name);
    const char *expr = c89_emit_rvalue(emit, hlir->value);
    const char *linkage = c89_get_linkage(attribs->linkage);

    WRITE_STRINGF(emit, "%s%s[1] = { %s };\n", linkage, type, expr);
}

static void c89_emit_globals(c89_emit_t *emit, size_t totalDecls, vector_t *modules)
{
    size_t totalModules = vector_len(modules);
    set_t *uniqueGlobals = set_new(totalDecls);

    for (size_t i = 0; i < totalModules; i++)
    {
        hlir_t *mod = vector_get(modules, i);

        for (size_t j = 0; j < vector_len(mod->globals); j++)
        {
            hlir_t *global = vector_get(mod->globals, j);
            if (set_check_ptr(uniqueGlobals, global))
            {
                continue;
            }

            c89_forward_global(emit, global);
        }
    }

    set_reset(uniqueGlobals);

    for (size_t i = 0; i < totalModules; i++)
    {
        hlir_t *mod = vector_get(modules, i);
        for (size_t j = 0; j < vector_len(mod->globals); j++)
        {
            hlir_t *global = vector_get(mod->globals, j);
            if (set_check_ptr(uniqueGlobals, global))
            {
                continue;
            }

            c89_emit_global(emit, global);
        }
    }
}

static const char *c89_fmt_params(c89_emit_t *emit, vector_t *params)
{
    size_t len = vector_len(params);
    if (len == 0)
    {
        return "";
    }

    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const hlir_t *param = vector_get(params, i);
        const char *type = c89_emit_type(emit, get_hlir_type(param), get_hlir_name(param));
        vector_set(result, i, (void *)type);
    }

    return str_join(", ", result);
}

static void c89_function_header(c89_emit_t *emit, const hlir_t *function)
{
    const hlir_attributes_t *attribs = get_hlir_attributes(function);
    vector_t *closureParams = closure_params(function);
    const char *name = "main";

    // TODO: return type cannot be array
    const char *params = "void";
    const char *linkage = "";

    if (attribs->linkage != eLinkEntryCli)
    {
        name = c89_mangle_name(emit, function);
        params = c89_fmt_params(emit, closureParams);
        linkage = c89_get_linkage(attribs->linkage);
    }

    const char *result = c89_emit_type(emit, closure_result(function), name);

    if (vector_len(closureParams) == 0)
    {
        const char *variaidc = closure_variadic(function) ? "" : "void";
        WRITE_STRINGF(emit, "%s%s(%s)", linkage, result, variaidc);
    }
    else
    {
        const char *variadic = closure_variadic(function) ? ", ..." : "";
        WRITE_STRINGF(emit, "%s%s(%s%s)", linkage, result, params, variadic);
    }
}

static void c89_forward_function(c89_emit_t *emit, const hlir_t *function)
{
    c89_function_header(emit, function);
    WRITE_STRING(emit, ";\n");
}

static void c89_emit_function(c89_emit_t *emit, const hlir_t *function)
{
    if (hlir_is_imported(function))
    {
        return;
    }

    c89_function_header(emit, function);

    WRITE_STRING(emit, "\n{\n");

    begin_indent(emit);

    vector_t *locals = function->locals;
    size_t numLocals = vector_len(locals);

    for (size_t i = 0; i < numLocals; i++)
    {
        hlir_t *local = vector_get(locals, i);
        const char *name = get_hlir_name(local);
        const char *type = c89_emit_type(emit, get_hlir_type(local), name);
        WRITE_STRINGF(emit, "%s[1];\n", type);
    }

    c89_emit_stmt(emit, function->body);

    end_indent(emit);

    WRITE_STRING(emit, "}\n");
}

static void c89_emit_functions(c89_emit_t *emit, size_t totalDecls, vector_t *modules)
{
    size_t totalModules = vector_len(modules);
    set_t *uniqueFunctions = set_new(totalDecls);

    for (size_t i = 0; i < totalModules; i++)
    {
        hlir_t *mod = vector_get(modules, i);

        for (size_t j = 0; j < vector_len(mod->functions); j++)
        {
            hlir_t *function = vector_get(mod->functions, j);
            if (set_check_ptr(uniqueFunctions, function))
            {
                continue;
            }

            c89_forward_function(emit, function);
        }
    }

    set_reset(uniqueFunctions);

    for (size_t i = 0; i < totalModules; i++)
    {
        hlir_t *mod = vector_get(modules, i);
        for (size_t j = 0; j < vector_len(mod->functions); j++)
        {
            hlir_t *function = vector_get(mod->functions, j);
            if (set_check_ptr(uniqueFunctions, function))
            {
                continue;
            }

            c89_emit_function(emit, function);
        }
    }
}

stream_t *c89_emit_modules(reports_t *reports, vector_t *modules)
{
    c89_emit_t emit = {.reports = reports, .output = stream_new(0x1000)};

    size_t totalDecls = 0;
    size_t totalModules = vector_len(modules);
    for (size_t i = 0; i < totalModules; i++)
    {
        const hlir_t *mod = vector_get(modules, i);
        size_t modTypes = vector_len(mod->types);
        size_t modGlobals = vector_len(mod->globals);
        size_t modFuncs = vector_len(mod->functions);

        totalDecls += modTypes + modGlobals + modFuncs;
    }

    // then use the total number of types to create a fast map
    emit.mangledNames = map_optimal(totalDecls);
    emit.depth = 0;

    c89_emit_types(&emit, modules);

    c89_emit_globals(&emit, totalDecls, modules);

    c89_emit_functions(&emit, totalDecls, modules);

    return emit.output;
}
