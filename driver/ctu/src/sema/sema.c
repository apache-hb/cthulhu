#include "sema.h"
#include "ast.h"
#include "attribs.h"
#include "cthulhu/hlir/arity.h"
#include "report/report.h"
#include "repr.h"
#include "suffix.h"

#include "cthulhu/hlir/digit.h"
#include "cthulhu/hlir/ops.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/decl.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/type.h"
#include "cthulhu/interface/runtime.h"

#include "base/macros.h"
#include "base/panic.h"
#include "base/memory.h"
#include "base/util.h"

#include "report/report-ext.h"

#include "std/set.h"
#include "std/str.h"
#include "std/map.h"

#include <stdio.h>

typedef struct
{
    size_t totalDecls;

    const hlir_t *currentImplicitType;

    vector_t *currentScope;

    hlir_t *currentFunction; // current function we are adding locals to
} sema_data_t;

static vector_t *enter_scope(sema_t *sema, size_t len)
{
    sema_data_t *data = sema_get_data(sema);

    vector_t *current = data->currentScope;
    data->currentScope = vector_new(len);

    return current;
}

static vector_t *leave_scope(sema_t *sema, vector_t *next)
{
    sema_data_t *data = sema_get_data(sema);

    vector_t *current = data->currentScope;
    data->currentScope = next;

    return current;
}

static void add_scope_step(sema_t *sema, hlir_t *hlir)
{
    sema_data_t *data = sema_get_data(sema);
    vector_push(&data->currentScope, hlir);
}

static const hlir_t *push_implicit_type(sema_t *sema, const hlir_t *type)
{
    sema_data_t *data = sema_get_data(sema);
    const hlir_t *current = data->currentImplicitType;
    data->currentImplicitType = type;
    return current;
}

static void pop_implicit_type(sema_t *sema, const hlir_t *type)
{
    sema_data_t *data = sema_get_data(sema);
    data->currentImplicitType = type;
}

static const hlir_t *get_implicit_type(sema_t *sema)
{
    sema_data_t *data = sema_get_data(sema);
    return data->currentImplicitType;
}

static void set_current_function(sema_t *sema, hlir_t *function)
{
    sema_data_t *data = sema_get_data(sema);
    data->currentFunction = function;
}

static hlir_t *get_current_function(sema_t *sema)
{
    sema_data_t *data = sema_get_data(sema);
    return data->currentFunction;
}

static const hlir_t *get_current_return_type(sema_t *sema)
{
    hlir_t *function = get_current_function(sema);
    CTASSERT(function != NULL);

    return function->result;
}

static sema_data_t *sema_data_new(void)
{
    sema_data_t *data = ctu_malloc(sizeof(sema_data_t));
    data->totalDecls = SIZE_MAX;
    data->currentImplicitType = NULL;
    data->currentScope = NULL;
    data->currentFunction = NULL;
    return data;
}

static sema_t *begin_sema(sema_t *parent, size_t *sizes)
{
    sema_data_t *data = parent != NULL ? sema_get_data(parent) : sema_data_new();
    sema_t *sema = sema_new(parent, eTagTotal, sizes);
    sema_set_data(sema, data);
    return sema;
}

static const char *kDigitNames[eSignTotal][eDigitTotal] = {
    [eSigned] =
        {
            [eDigitChar] = "char",
            [eDigitShort] = "short",
            [eDigitInt] = "int",
            [eDigitLong] = "long",

            [eDigitPtr] = "intptr",
            [eDigitSize] = "isize",
            [eDigitMax] = "intmax",
        },
    [eUnsigned] =
        {
            [eDigitChar] = "uchar",
            [eDigitShort] = "ushort",
            [eDigitInt] = "uint",
            [eDigitLong] = "ulong",

            [eDigitPtr] = "uintptr",
            [eDigitSize] = "usize",
            [eDigitMax] = "uintmax",
        },
};

static hlir_t *kVoidType = NULL;
static hlir_t *kEmptyType = NULL;
static hlir_t *kOpaqueType = NULL;
static hlir_t *kBoolType = NULL;
static hlir_t *kStringType = NULL;
static hlir_t *kFloatType = NULL;
static hlir_t *kDigitTypes[eSignTotal * eDigitTotal];

static hlir_t *kVaListType = NULL;

static hlir_t *kUnresolvedType = NULL;
static hlir_t *kUninitalized = NULL;

static hlir_t *kZeroIndex = NULL;

#define DIGIT_INDEX(sign, digit) ((sign)*eDigitTotal + (digit))

hlir_t *get_digit_type(sign_t sign, digit_t digit)
{
    return kDigitTypes[DIGIT_INDEX(sign, digit)];
}

static const char *get_digit_name(sign_t sign, digit_t digit)
{
    return kDigitNames[sign][digit];
}

static sema_t *kRootSema = NULL;

static const hlir_t *get_common_type(node_t *node, const hlir_t *lhs, const hlir_t *rhs);

static bool is_aggregate(const hlir_t *type)
{
    return hlir_is(type, eHlirStruct) || hlir_is(type, eHlirUnion);
}

static bool is_ptr(const hlir_t *type)
{
    return hlir_is(type, eHlirPointer);
}

static bool is_indexable(const hlir_t *type)
{
    const hlir_t *real = hlir_follow_type(type);
    if (is_ptr(real))
    {
        return real->indexable;
    }

    return hlir_is(real, eHlirArray);
}

static bool is_lvalue(const hlir_t *expr)
{
    hlir_kind_t kind = get_hlir_kind(expr);
    switch (kind)
    {
    case eHlirLoad:
    case eHlirAccess:
    case eHlirIndex:
    case eHlirLocal:
    case eHlirGlobal:
        return true;

    default:
        return false;
    }
}

static const hlir_t *common_pointer_type(node_t *node, const hlir_t *lhs, const hlir_t *rhs)
{
    if (!hlir_is(lhs, eHlirPointer) || !hlir_is(rhs, eHlirPointer))
    {
        return hlir_error(node, "no common pointer type");
    }

    if (lhs == rhs)
    {
        return lhs;
    }

    return hlir_error(node, "no common type between unrelated pointers");
}

static const hlir_t *get_common_type(node_t *node, const hlir_t *lhs, const hlir_t *rhs)
{
    const hlir_t *lhsType = hlir_follow_type(lhs);
    const hlir_t *rhsType = hlir_follow_type(rhs);

    if (hlir_is(lhsType, eHlirDigit) && hlir_is(rhsType, eHlirDigit))
    {
        // get the largest size
        digit_t width = MAX(lhsType->width, rhsType->width);
        // if either is signed the result is signed
        sign_t sign = (lhsType->sign == eSigned || rhsType->sign == eSigned) ? eSigned : eUnsigned;

        return get_digit_type(sign, width);
    }

    if (hlir_is(lhsType, eHlirBool) && hlir_is(rhsType, eHlirBool))
    {
        return kBoolType;
    }

    if (hlir_is(lhsType, eHlirOpaque) && hlir_is(rhsType, eHlirOpaque))
    {
        return kOpaqueType;
    }

    if (hlir_is(lhsType, eHlirOpaque) && hlir_is(rhsType, eHlirPointer))
    {
        return kOpaqueType;
    }

    if (hlir_is(lhsType, eHlirPointer) && hlir_is(rhsType, eHlirOpaque))
    {
        return kOpaqueType;
    }

    // doesnt account for const, probably wrong
    const hlir_t *commonPtr = common_pointer_type(node, lhsType, rhsType);
    if (!hlir_is(commonPtr, eHlirError))
    {
        return commonPtr;
    }

    return hlir_error(node, "unknown common type");
}

#define IS_OPAQUE(it) (hlir_is(it, eHlirOpaque))
#define IS_PTR(it) (hlir_is(it, eHlirPointer))
#define IS_STR(it) (hlir_is(it, eHlirString))

static bool is_pointer_type(const hlir_t *hlir)
{
    return IS_OPAQUE(hlir) || IS_PTR(hlir) || IS_STR(hlir);
}

static hlir_t *ptr_convert_to(hlir_t *expr, const hlir_t *type, const hlir_t *ptrDst, const hlir_t *ptrSrc)
{
    if (!is_pointer_type(ptrDst) || !is_pointer_type(ptrSrc))
    {
        return NULL;
    }

    if (hlir_types_equal(ptrDst, ptrSrc))
    {
        return expr;
    }

    if (!IS_OPAQUE(ptrDst) && !IS_OPAQUE(ptrSrc))
    {
        return ptr_convert_to(expr, type, ptrDst->ptr, ptrSrc->ptr);
    }

    if (IS_OPAQUE(ptrDst) || IS_OPAQUE(ptrSrc))
    {
        return hlir_cast(type, expr, eCastBit);
    }

    return NULL;
}

static hlir_t *convert_to(reports_t *reports, const hlir_t *to, hlir_t *expr)
{
    const hlir_t *dstType = hlir_follow_type(to);
    const hlir_t *srcType = hlir_follow_type(get_hlir_type(expr));

    // if both types are equal no conversion is needed
    if (hlir_types_equal(dstType, srcType))
    {
        return expr;
    }

    hlir_t *ptrType = ptr_convert_to(expr, dstType, dstType, srcType);
    if (ptrType != NULL)
    {
        return ptrType;
    }

    // TODO: this doesnt handle warning on narrowing and other cases
    // dont use get_common_type here
    const hlir_t *common = get_common_type(get_hlir_node(expr), dstType, srcType);
    if (!hlir_is(common, eHlirError))
    {
        return hlir_cast(common, expr, eCastSignExtend);
    }

    report(reports, eFatal, get_hlir_node(expr), "cannot convert from %s to %s",
        ctu_repr(reports, expr, true),
        ctu_type_repr(reports, to, true)
    );
    return hlir_error(get_hlir_node(expr), "failed to convert type");
}

static bool is_discard_ident(const char *id)
{
    return id == NULL || str_equal(id, "$");
}

static void add_decl(sema_t *sema, int tag, const char *name, hlir_t *decl)
{
    node_t *node = get_hlir_node(decl);

    if (is_discard_ident(name))
    {
        report(sema_reports(sema), eFatal, node, "discarding declaration");
        return;
    }

    // was this already declared?
    hlir_t *other = sema_get(sema, tag, name);
    if (other != NULL)
    {
        // if it was report it and dont add this new one
        node_t *otherNode = get_hlir_node(other);
        report_shadow(sema_reports(sema), name, otherNode, node);
        return;
    }

    sema_set(sema, tag, name, decl);
}

static void add_basic_types(sema_t *sema)
{
    add_decl(sema, eSemaTypes, "void", kVoidType);
    add_decl(sema, eSemaTypes, "bool", kBoolType);
    add_decl(sema, eSemaTypes, "str", kStringType);
    add_decl(sema, eSemaTypes, "float", kFloatType);

    // noreturn type for exit/abort/terminate etc
    add_decl(sema, eSemaTypes, "noreturn", kEmptyType);
    add_decl(sema, eSemaTypes, "opaque", kOpaqueType); // equivalent to void*

    add_decl(sema, eSemaTypes, "valist", kVaListType);

    for (int sign = 0; sign < eSignTotal; sign++)
    {
        for (int digit = 0; digit < eDigitTotal; digit++)
        {
            const char *name = get_digit_name(sign, digit);
            hlir_t *type = get_digit_type(sign, digit);

            add_decl(sema, eSemaTypes, name, type);
        }
    }

    // enable the below later

    // special types for interfacing with C
    // add_decl(sema, eTagTypes, "enum", hlir_digit(node, "enum", eInt, eSigned));
}

void ctu_init_compiler(runtime_t *runtime)
{
    size_t sizes[eTagTotal] = {
        [eSemaValues] = 1, [eSemaProcs] = 1, [eSemaTypes] = 32, [eSemaModules] = 1, [eTagAttribs] = 1, [eTagSuffix] = 32,
    };

    kRootSema = sema_root_new(runtime->reports, eTagTotal, sizes);

    node_t *node = node_builtin();

    kVoidType = hlir_unit(node, "void");
    kEmptyType = hlir_empty(node, "noreturn");
    kOpaqueType = hlir_opaque(node, "opaque");
    kBoolType = hlir_bool(node, "bool");
    kStringType = hlir_string(node, "str");
    kFloatType = hlir_decimal(node, "float");

    kVaListType = hlir_va_list(node, "valist");

    for (int sign = 0; sign < eSignTotal; sign++)
    {
        for (int digit = 0; digit < eDigitTotal; digit++)
        {
            const char *name = get_digit_name(sign, digit);
            kDigitTypes[DIGIT_INDEX(sign, digit)] = hlir_digit(node, name, digit, sign);
        }
    }

    add_basic_types(kRootSema);
    add_builtin_suffixes(kRootSema);

    kUnresolvedType = hlir_error(node, "unresolved");
    kUninitalized = hlir_error(node, "uninitialized");

    mpz_t zero;
    mpz_init_set_ui(zero, 0);

    kZeroIndex = hlir_digit_literal(node, get_digit_type(eUnsigned, eDigitSize), zero);
}

static hlir_t *sema_type(sema_t *sema, ast_t *ast);

static hlir_t *sema_expr(sema_t *sema, ast_t *ast);

// hacky as hell but seems to work fine
static hlir_t *sema_lvalue(sema_t *sema, ast_t *ast)
{
    hlir_t *result = sema_expr(sema, ast);
    if (hlir_is(result, eHlirLoad))
    {
        result = result->read;
    }
    return result;
}

// also hacky and weird, whatever
static hlir_t *sema_rvalue(sema_t *sema, ast_t *ast)
{
    hlir_t *result = sema_expr(sema, ast);
    if (hlir_is(result, eHlirAccess) || hlir_is(result, eHlirIndex))
    {
        result = hlir_name(get_hlir_node(result), result);
    }
    return result;
}

static hlir_t *sema_rvalue_with_implicit(sema_t *sema, ast_t *ast, const hlir_t *type)
{
    const hlir_t *save = push_implicit_type(sema, type);
    hlir_t *result = sema_rvalue(sema, ast);
    pop_implicit_type(sema, save);
    return result;
}

static sema_t *sema_path(sema_t *sema, vector_t *path, node_t *node)
{
    size_t len = vector_len(path);
    sema_t *current = sema;
    for (size_t i = 0; i < len - 1; i++)
    {
        const char *name = vector_get(path, i);
        if (name == NULL)
        {
            report(sema_reports(sema), eFatal, node, "discarded path segment");
            return NULL;
        }

        sema_t *next = sema_get(current, eSemaModules, name);

        if (next == NULL)
        {
            report(sema_reports(sema), eFatal, node, "unknown namespace `%s`", name);
            return NULL;
        }

        current = next;
    }

    return current;
}

static hlir_t *begin_function(sema_t *sema, ast_t *ast);

typedef struct
{
    const hlir_t *type;
    hlir_t *init;
} sema_value_t;

static sema_value_t sema_value(sema_t *sema, ast_t *stmt);

static hlir_t *sema_global(sema_t *sema, ast_t *ast)
{
    sema_value_t result = sema_value(sema, ast);

    if (result.init != NULL && hlir_is(result.init, eHlirError))
    {
        return result.init;
    }

    return hlir_global(ast->node, ast->name, result.type, result.init);
}

static hlir_t *sema_alias(sema_t *sema, ast_t *ast)
{
    hlir_t *type = sema_type(sema, ast->alias);
    return hlir_alias(ast->node, ast->name, type);
}

static hlir_t *begin_type_resolve(sema_t *sema, void *user)
{
    ast_t *ast = user;

    switch (ast->of)
    {
    case eAstDeclUnion:
        return hlir_begin_union(ast->node, ast->name);

    case eAstDeclAlias:
        return sema_alias(sema, ast);

    case eAstDeclVariant:
    case eAstDeclStruct:
        return hlir_begin_struct(ast->node, ast->name);

    case eAstFunction:
        return begin_function(sema, ast);

    case eAstVariable:
        return sema_global(sema, ast);

    default:
        report(sema_reports(sema), eInternal, ast->node, "unknown ast type in resolution");
        return hlir_error(ast->node, "unknown type resolution");
    }
}

void check_valid_import(sema_t *sema, sema_t *cur, ast_t *ast, hlir_t *hlir)
{
    const char *name = vector_tail(ast->path);
    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);
    if (sema != cur && attribs->visibility != eVisiblePublic)
    {
        message_t *id =
            report(sema_reports(sema), eFatal, ast->node, "symbol '%s' is not visible inside this context", name);
        report_append(id, get_hlir_node(hlir), "originally declared here");
    }
}

static hlir_t *sema_get_resolved(sema_t *sema, size_t tag, const char *name)
{
    hlir_t *it = sema_get(sema, tag, name);
    if (it != NULL)
    {
        return sema_resolve(kRootSema, it, begin_type_resolve);
    }
    return NULL;
}

static hlir_t *sema_typename(sema_t *sema, ast_t *ast)
{
    sema_t *current = sema_path(sema, ast->path, ast->node);

    const char *name = vector_tail(ast->path);
    while (current != NULL)
    {
        hlir_t *decl = sema_get_resolved(current, eSemaTypes, name);
        if (decl != NULL)
        {
            check_valid_import(sema, current, ast, decl);
            return decl;
        }

        current = sema_parent(current);
    }

    report(sema_reports(sema), eFatal, ast->node, "type '%s' not found", name);
    return hlir_error(ast->node, "type not found");
}

static hlir_t *sema_pointer(sema_t *sema, ast_t *ast)
{
    hlir_t *type = sema_type(sema, ast->type);
    const hlir_t *real = hlir_follow_type(type);

    if (real == kVoidType)
    {
        report(sema_reports(sema), eWarn, ast->node, "pointers to `%s` are deprecated, please use `%s` instead", get_hlir_name(real), get_hlir_name(kOpaqueType));
        return kOpaqueType;
    }

    if (real == kEmptyType || real == kVaListType)
    {
        report(sema_reports(sema), eFatal, ast->node, "pointers to `%s` are not allowed", get_hlir_name(real));
    }

    return hlir_pointer(ast->node, NULL, type, ast->indexable);
}

static hlir_t *sema_array(sema_t *sema, ast_t *ast)
{
    hlir_t *size = sema_rvalue(sema, ast->size);
    hlir_t *type = sema_type(sema, ast->type);

    return hlir_array(sema_reports(sema), ast->node, NULL, type, size);
}

static hlir_t *sema_closure(sema_t *sema, ast_t *ast)
{
    hlir_t *result = sema_type(sema, ast->result);
    size_t len = vector_len(ast->params);
    vector_t *params = vector_of(len);

    for (size_t i = 0; i < len; i++)
    {
        ast_t *param = vector_get(ast->params, i);
        hlir_t *type = sema_type(sema, param);
        vector_set(params, i, type);

        if (hlir_is(type, eHlirUnit) || hlir_is(type, eHlirEmpty))
        {
            report(sema_reports(sema), eFatal, param->node, "void parameter");
        }
    }

    return hlir_closure(ast->node, NULL, params, result);
}

static hlir_t *sema_type(sema_t *sema, ast_t *ast)
{
    switch (ast->of)
    {
    case eAstTypename:
        return sema_typename(sema, ast);
    case eAstPointer:
        return sema_pointer(sema, ast);
    case eAstArray:
        return sema_array(sema, ast);
    case eAstClosure:
        return sema_closure(sema, ast);
    default:
        report(sema_reports(sema), eInternal, ast->node, "unknown sema-type: %d", ast->of);
        return hlir_error(ast->node, "unknown sema-type");
    }
}

static hlir_t *sema_digit(sema_t *sema, ast_t *ast)
{
    // TODO: maybe we want untyped integer literals
    suffix_t *suffix = sema_get(sema, eTagSuffix, ast->suffix);
    if (suffix == NULL)
    {
        report(sema_reports(sema), eFatal, ast->node, "invalid suffix '%s'", ast->suffix);
        return hlir_error(ast->node, "invalid suffix");
    }

    return apply_suffix(sema, ast, suffix);
}

static hlir_t *sema_decimal(ast_t *ast)
{
    return hlir_decimal_literal(ast->node, kFloatType, ast->decimal);
}

static hlir_t *sema_bool(ast_t *ast)
{
    return hlir_bool_literal(ast->node, kBoolType, ast->boolean);
}

static hlir_t *sema_string(ast_t *ast)
{
    struct string_view_t literal = { .data = ast->string, .size = ast->length };
    return hlir_string_literal(ast->node, kStringType, literal);
}

static hlir_t *sema_unary_digit(sema_t *sema, ast_t *ast, hlir_t *operand)
{
    const hlir_t *type = get_hlir_type(operand);
    const hlir_t *realType = hlir_real_type(type);

    if (!hlir_is(realType, eHlirDigit) && !hlir_is(realType, eHlirDecimal))
    {
        report(sema_reports(sema), eFatal, ast->node, "cannot perform unary operation on '%s'",
               ctu_repr(sema_reports(sema), operand, true));
    }

    return hlir_unary(ast->node, ast->unary, operand);
}

static hlir_t *sema_unary_bool(sema_t *sema, ast_t *ast, hlir_t *operand)
{
    const hlir_t *type = get_hlir_type(operand);
    const hlir_t *realType = hlir_real_type(type);

    if (!hlir_is(realType, eHlirBool))
    {
        report(sema_reports(sema), eFatal, ast->node, "cannot perform boolean unary operation on '%s'",
               ctu_repr(sema_reports(sema), operand, true));
    }

    return hlir_unary(ast->node, ast->unary, operand);
}

static hlir_t *sema_unary(sema_t *sema, ast_t *ast)
{
    hlir_t *operand = sema_rvalue(sema, ast->operand);

    switch (ast->unary)
    {
    case eUnaryAbs:
    case eUnaryNeg:
    case eUnaryFlip:
        return sema_unary_digit(sema, ast, operand);

    case eUnaryNot:
        return sema_unary_bool(sema, ast, operand);

    default:
        report(sema_reports(sema), eInternal, ast->node, "unexpected unary operand %s", unary_name(ast->unary));
        return hlir_error(ast->node, "unsupported unary operand");
    }
}

static hlir_t *sema_binary(sema_t *sema, ast_t *ast)
{
    hlir_t *lhs = sema_rvalue(sema, ast->lhs);
    hlir_t *rhs = sema_rvalue(sema, ast->rhs);

    const hlir_t *type = get_common_type(ast->node, get_hlir_type(lhs), get_hlir_type(rhs));

    if (!hlir_is(type, eHlirDigit))
    {
        message_t *id = report(sema_reports(sema), eFatal, ast->node, "cannot perform binary operations on %s",
                               ctu_type_repr(sema_reports(sema), type, true));
        report_append(id, get_hlir_node(lhs), "%s", ctu_repr(sema_reports(sema), lhs, false));
        report_append(id, get_hlir_node(rhs), "%s", ctu_repr(sema_reports(sema), rhs, false));
    }

    return hlir_binary(ast->node, type, ast->binary, lhs, rhs);
}

static hlir_t *sema_compare(sema_t *sema, ast_t *ast)
{
    hlir_t *lhs = sema_rvalue(sema, ast->lhs);
    hlir_t *rhs = sema_rvalue(sema, ast->rhs);

    const hlir_t *type = get_common_type(ast->node, get_hlir_type(lhs), get_hlir_type(rhs));

    if (hlir_is(type, eHlirError))
    {
        message_t *id = report(sema_reports(sema), eFatal, ast->node, "cannot perform comparison operations on %s",
                               ctu_type_repr(sema_reports(sema), type, true));
        report_append(id, get_hlir_node(lhs), "%s", ctu_repr(sema_reports(sema), lhs, false));
        report_append(id, get_hlir_node(rhs), "%s", ctu_repr(sema_reports(sema), rhs, false));
    }

    return hlir_compare(ast->node, kBoolType, ast->compare, lhs, rhs);
}

static hlir_t *sema_ident(sema_t *sema, ast_t *ast)
{
    sema_t *current = sema_path(sema, ast->path, ast->node);
    if (current == NULL)
    {
        return hlir_error(ast->node, "failed to resolve namespace");
    }

    const char *name = vector_tail(ast->path);

    if (name == NULL)
    {
        report(sema_reports(sema), eFatal, ast->node, "cannot resolve discarded identifier");
        return hlir_error(ast->node, "discarded indentifier");
    }

    hlir_t *var = sema_get_resolved(current, eSemaValues, name);
    if (var != NULL)
    {
        check_valid_import(sema, current, ast, var);
        return hlir_name(ast->node, var);
    }

    hlir_t *func = sema_get_resolved(current, eSemaProcs, name);
    if (func != NULL)
    {
        check_valid_import(sema, current, ast, func);
        return func;
    }

    report(sema_reports(sema), eFatal, ast->node, "unknown identifier '%s'", name);
    return hlir_error(ast->node, "unknown identifier");
}

static char *min_param_msg(const hlir_t *fn)
{
    size_t len = vector_len(closure_params(fn));
    bool variadic = closure_variadic(fn);
    if (variadic)
    {
        return format("at least %zu parameter%s", len - 1, len == 2 ? "" : "s");
    }

    return format("%zu parameter%s", len, len == 1 ? "" : "s");
}

static hlir_t *sema_call(sema_t *sema, ast_t *ast)
{
    hlir_t *call = sema_rvalue(sema, ast->call);
    const hlir_t *fnType = get_hlir_type(call);
    if (!hlir_is(fnType, eHlirFunction) && !hlir_is(fnType, eHlirClosure))
    {
        message_t *id = report(sema_reports(sema), eFatal, ast->node, "can only call function types");
        report_underline(id, "%s", ctu_repr(sema_reports(sema), call, true));
        return hlir_error(ast->node, "invalid callable");
    }

    size_t len = vector_len(ast->args);
    vector_t *params = closure_params(fnType);
    bool variadic = closure_variadic(fnType);

    size_t totalParams = vector_len(params);

    if (len != totalParams)
    {
        if (len < totalParams)
        {
            message_t *id = report(sema_reports(sema), eFatal, ast->node, "not enough parameters specified");
            report_note(id, "expected %s got `%zu` instead", min_param_msg(fnType), len);
            return hlir_error(ast->node, "incorrect argument count");
        }

        if (!variadic)
        {
            message_t *id = report(sema_reports(sema), eFatal, ast->node, "too many parameters specified");
            report_note(id, "expected %s got `%zu` instead", min_param_msg(fnType), len);
            return hlir_error(ast->node, "incorrect argument count");
        }
    }

    vector_t *args = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        ast_t *arg = vector_get(ast->args, i);
        hlir_t *expectedType = (i >= totalParams) 
            ? NULL 
            : vector_get(params, i);

        hlir_t *hlir = sema_rvalue_with_implicit(sema, arg, expectedType);

        if (i >= totalParams)
        {
            CTASSERT(variadic);
            vector_set(args, i, hlir);
        }
        else
        {
            const hlir_t *cast = convert_to(sema_reports(sema), expectedType, hlir);
            vector_set(args, i, (hlir_t*)cast);
        }
    }

    return hlir_call(ast->node, call, args);
}

static hlir_t *sema_null(ast_t *ast)
{
    hlir_t *zero = hlir_int_literal(ast->node, get_digit_type(eUnsigned, eDigitPtr), 0);
    return hlir_cast(kOpaqueType, zero, eCastBit);
}

static hlir_t *sema_access(sema_t *sema, ast_t *ast)
{
    hlir_t *object = sema_lvalue(sema, ast->record);
    const hlir_t *objectType = get_hlir_type(object);
    if (is_ptr(objectType))
    {
        if (!ast->indirect)
        {
            report(sema_reports(sema), eWarn, ast->node, "accessing pointer without dereferencing");
        }

        objectType = objectType->ptr;
    }

    if (!is_aggregate(objectType))
    {
        report(sema_reports(sema), eFatal, ast->node, "cannot access non-aggregate type");
        return hlir_error(ast->node, "invalid access");
    }

    vector_t *fields = objectType->fields;
    for (size_t i = 0; i < vector_len(fields); i++)
    {
        hlir_t *field = vector_get(fields, i);
        if (str_equal(field->name, ast->access))
        {
            return hlir_access(ast->node, object, field);
        }
    }

    report(sema_reports(sema), eFatal, ast->node, "unknown field '%s'", ast->access);
    return hlir_error(ast->node, "unknown field");
}

static hlir_t *sema_ref(sema_t *sema, ast_t *ast)
{
    hlir_t *expr = sema_lvalue(sema, ast->operand);
    if (!is_lvalue(expr))
    {
        report(sema_reports(sema), eFatal, ast->node, "cannot take reference of non-lvalue");
        return hlir_error(ast->node, "invalid ref");
    }

    return hlir_addr(ast->node, expr);
}

static hlir_t *sema_sizeof(sema_t *sema, ast_t *ast)
{
    hlir_t *type = sema_type(sema, ast->type);
    return hlir_builtin(ast->node, get_digit_type(eUnsigned, eDigitSize), type, eBuiltinSizeOf);
}

static hlir_t *sema_index(sema_t *sema, ast_t *ast)
{
    hlir_t *array = sema_lvalue(sema, ast->array);
    hlir_t *index = sema_rvalue(sema, ast->index);

    const hlir_t *arrayType = get_hlir_type(array);
    const hlir_t *indexType = get_hlir_type(index);

    if (!is_indexable(arrayType))
    {
        report(sema_reports(sema), eFatal, ast->node, "cannot index non-indexable type");
        return hlir_error(ast->node, "invalid index");
    }

    const hlir_t *sizeType = get_digit_type(eUnsigned, eDigitSize);
    const hlir_t *common = get_common_type(get_hlir_node(index), indexType, sizeType);
    if (hlir_is(common, eHlirError))
    {
        report(sema_reports(sema), eFatal, ast->node, "index must be convertable to size type");
        return hlir_error(ast->node, "invalid index type");
    }

    hlir_t *cast = hlir_cast(sizeType, index, eCastSignExtend);
    return hlir_index(ast->node, array, cast);
}

static hlir_t *struct_get_field(const hlir_t *aggregate, const char *name)
{
    CTASSERT(hlir_is(aggregate, eHlirStruct));

    vector_t *fields = aggregate->fields;
    for (size_t i = 0; i < vector_len(fields); i++)
    {
        hlir_t *field = vector_get(fields, i);
        printf("names: %s = %s\n", field->name, name);
        if (str_equal(get_hlir_name(field), name))
        {
            return field;
        }
    }

    return NULL;
}

/**
 * @brief Selects the implicit type for an initializer.
 * @param sema The semantic analyzer.
 * @param context The implicit expected type.
 * @param user The user-provided initializer type.
 */
static const hlir_t *select_implicit_init_type(sema_t *sema, const hlir_t *context, ast_t *user)
{
    if (user == NULL)
    {
        return context;
    }

    return sema_type(sema, user);
}

static void add_default_inits(sema_t *sema, node_t *node, hlir_t *local, const hlir_t *type, set_t *fields)
{
    size_t len = vector_len(type->fields);
    set_t *missing = set_new(len);

    for (size_t i = 0; i < len; i++)
    {
        hlir_t *field = vector_get(type->fields, i);
        if (set_contains(fields, field->name))
        {
            continue;
        }

        set_add(missing, field->name);

        hlir_t *access = hlir_access(node, local, field);
        hlir_t *assign = hlir_assign(node, access, NULL); // TODO: default value
        add_scope_step(sema, assign);
    }

    if (!set_empty(missing))
    {
        message_t *id = report(sema_reports(sema), eWarn, node, "missing initializers for fields");
        set_iter_t iter = set_iter(missing);
        while (set_has_next(&iter))
        {
            const char *name = set_next(&iter);
            report_append(id, node, "field: %s", name);
        }
    }
}

static hlir_t *sema_init(sema_t *sema, ast_t *ast)
{
    const hlir_t *expected = get_implicit_type(sema);
    if (expected == NULL && ast->type == NULL)
    {
        report(sema_reports(sema), eInternal, ast->node, "cannot infer type of initializer in current context");
        return hlir_error(ast->node, "invalid initializer");
    }

    const hlir_t *type = select_implicit_init_type(sema, expected, ast->type);

    if (hlir_is(type, eHlirError))
    {
        return hlir_error(ast->node, "invalid initializer type");
    }

    const hlir_t *real = hlir_follow_type(type);

    if (!hlir_is(real, eHlirStruct))
    {
        report(sema_reports(sema), eFatal, ast->node, "cannot initialize non-struct type (yet)");
        return hlir_error(ast->node, "invalid initializer type");
    }

    hlir_t *local = hlir_local(ast->node, NULL, real);

    hlir_add_local(get_current_function(sema), local);

    set_t *setFields = set_new(vector_len(real->fields));

    size_t len = vector_len(ast->inits);
    for (size_t i = 0; i < len; i++)
    {
        ast_t *field = vector_get(ast->inits, i);
        CTASSERT(field->of == eAstFieldInit);

        hlir_t *it = struct_get_field(real, field->name);

        if (it == NULL)
        {
            report(sema_reports(sema), eFatal, ast->node, "unknown init field '%s'", field->name);
            return hlir_error(ast->node, "invalid initializer");
        }

        if (set_contains(setFields, field->name))
        {
            report(sema_reports(sema), eFatal, ast->node, "duplicate field '%s'", field->name);
            return hlir_error(ast->node, "invalid initializer");
        }

        set_add(setFields, field->name);

        hlir_t *value = sema_rvalue(sema, field->value);

        hlir_t *access = hlir_access(field->node, local, it);
        hlir_t *assign = hlir_assign(field->node, access, value);

        add_scope_step(sema, assign);
    }

    add_default_inits(sema, ast->node, local, real, setFields);

    return hlir_name(ast->node, local);
}

static hlir_t *sema_deref(sema_t *sema, ast_t *ast)
{
    hlir_t *expr = sema_rvalue(sema, ast->operand);

    const hlir_t *ty = get_hlir_type(expr);
    if (!hlir_is(ty, eHlirPointer))
    {
        report(sema_reports(sema), eFatal, ast->node, "cannot dereference non-pointer type");
        return hlir_error(ast->node, "invalid dereference");
    }

    return hlir_index(ast->node, expr, kZeroIndex);
}

static hlir_t *sema_expr(sema_t *sema, ast_t *ast)
{
    switch (ast->of)
    {
    case eAstDigit:
        return sema_digit(sema, ast);
    case eAstDecimal:
        return sema_decimal(ast);
    case eAstBool:
        return sema_bool(ast);
    case eAstString:
        return sema_string(ast);
    case eAstNull:
        return sema_null(ast);
    case eAstUnary:
        return sema_unary(sema, ast);
    case eAstBinary:
        return sema_binary(sema, ast);
    case eAstCompare:
        return sema_compare(sema, ast);
    case eAstName:
        return sema_ident(sema, ast);
    case eAstCall:
        return sema_call(sema, ast);
    case eAstAccess:
        return sema_access(sema, ast);
    case eAstRef:
        return sema_ref(sema, ast);
    case eAstSizeOf:
        return sema_sizeof(sema, ast);
    case eAstIndex:
        return sema_index(sema, ast);
    case eAstDeref:
        return sema_deref(sema, ast);

    case eAstInit:
        return sema_init(sema, ast);

    default:
        report(sema_reports(sema), eInternal, ast->node, "unknown sema-expr: %d", ast->of);
        return hlir_error(ast->node, "unknown sema-expr");
    }
}

static hlir_t *sema_stmt(sema_t *sema, ast_t *stmt);

static hlir_t *sema_stmts(sema_t *sema, ast_t *stmts)
{
    size_t len = vector_len(stmts->stmts);

    vector_t *upper = enter_scope(sema, len);

    for (size_t i = 0; i < len; i++)
    {
        ast_t *stmt = vector_get(stmts->stmts, i);
        hlir_t *hlir = sema_stmt(sema, stmt);
        add_scope_step(sema, hlir);
    }

    return hlir_stmts(stmts->node, leave_scope(sema, upper));
}

static hlir_t *sema_return(sema_t *sema, ast_t *ast)
{
    hlir_t *result = sema_rvalue_with_implicit(sema, ast->operand, get_current_return_type(sema));

    return hlir_return(ast->node, result);
}

static hlir_t *sema_while(sema_t *sema, ast_t *ast)
{
    if (ast->label != NULL)
    {
        report(sema_reports(sema), eInternal, ast->node, "loop labels not yet supported");
    }

    hlir_t *cond = sema_rvalue(sema, ast->cond);

    size_t sizes[eTagTotal] = {
        [eSemaValues] = 32,
        [eSemaProcs] = 4,
        [eSemaTypes] = 4,
        [eSemaModules] = 1,
        [eTagAttribs] = 1,
        [eTagSuffix] = 1,
    };

    sema_t *nestThen = begin_sema(sema, sizes);

    hlir_t *then = sema_stmt(nestThen, ast->then);
    hlir_t *other = NULL;

    if (ast->other != NULL)
    {
        sema_t *nextOther = begin_sema(sema, sizes);
        other = sema_stmt(nextOther, ast->other);
    }

    const hlir_t *condType = get_hlir_type(cond);
    if (!hlir_types_equal(condType, kBoolType))
    {
        message_t *id = report(sema_reports(sema), eFatal, get_hlir_node(cond), "loop condition must be boolean");
        report_note(id, "type '%s' found", ctu_repr(sema_reports(sema), cond, true));
        return hlir_error(ast->node, "invalid loop condition");
    }

    return hlir_loop(ast->node, cond, then, other);
}

static const hlir_t *sema_global_type(sema_t *sema, ast_t *expected, const hlir_t *expr)
{
    if (expected == NULL)
    {
        return get_hlir_type(expr);
    }
    else if (expected->of == eAstVarArgs)
    {
        report(sema_reports(sema), eFatal, expected->node, "cannot use varargs as a global type");
        return hlir_error(expected->node, "invalid global type");
    }
    else
    {
        return sema_type(sema, expected);
    }
}

static sema_value_t sema_value(sema_t *sema, ast_t *stmt)
{
    if (stmt->expected != NULL && stmt->init != NULL)
    {
        const hlir_t *type = sema_global_type(sema, stmt->expected, NULL);
        hlir_t *init = sema_rvalue_with_implicit(sema, stmt->init, type);

        sema_value_t result = {type, init};
        return result;
    }


    // TODO: this is getting ugly
    hlir_t *init = stmt->init != NULL
        ? sema_rvalue(sema, stmt->init)
        : NULL;

    const hlir_t *type = sema_global_type(sema, stmt->expected, init);

    sema_value_t result = {type, init};

    if ((stmt->init != NULL && stmt->expected != NULL) )
    {
        const hlir_t *convert = convert_to(sema_reports(sema), type, init);
        if (!hlir_is(convert, eHlirError))
        {
            result.init = (hlir_t*)convert; // TODO: evil
            return result;
        }
        message_t *id = report(sema_reports(sema), eFatal, stmt->node, "incompatible initializer and explicit type");
        report_underline(id, "found '%s', expected '%s'", ctu_type_repr(sema_reports(sema), type, true),
                         ctu_repr(sema_reports(sema), init, true));
        result.init = hlir_error(stmt->node, "invalid value declaration");
        return result;
    }

    if (!stmt->mut)
    {
        result.type = hlir_qualified(type, hlir_tags(eQualConst));
    }

    return result;
}

static hlir_t *sema_local(sema_t *sema, ast_t *stmt)
{
    sema_value_t value = sema_value(sema, stmt);

    if (value.init != NULL && hlir_is(value.init, eHlirError))
    {
        return value.init;
    }

    hlir_t *local = hlir_local(stmt->node, stmt->name, value.type);
    add_decl(sema, eSemaValues, stmt->name, local);
    hlir_add_local(get_current_function(sema), local);

    if (value.init != NULL)
    {
        return hlir_assign(get_hlir_node(value.init), local, value.init);
    }
    else
    {
        return hlir_stmts(stmt->node, vector_new(0));
    }
}

static hlir_t *sema_break(sema_t *sema, ast_t *stmt)
{
    if (stmt->label != NULL)
    {
        report(sema_reports(sema), eInternal, stmt->node, "loop labels not yet supported"); // TODO: support labels
    }

    return hlir_break(stmt->node, NULL);
}

static hlir_t *sema_continue(sema_t *sema, ast_t *stmt)
{
    if (stmt->label != NULL)
    {
        report(sema_reports(sema), eInternal, stmt->node, "loop labels not yet supported"); // TODO: support labels
    }

    return hlir_continue(stmt->node, NULL);
}

static hlir_t *sema_branch(sema_t *sema, ast_t *stmt)
{
    hlir_t *cond = sema_rvalue(sema, stmt->cond);

    size_t tags[eTagTotal] = {
        [eSemaValues] = 32,
        [eSemaProcs] = 32,
        [eSemaTypes] = 32,
        [eSemaModules] = 32,
        [eTagAttribs] = 32,
    };

    sema_t *bodyInner = begin_sema(sema, tags);
    sema_t *otherInner = begin_sema(sema, tags);

    hlir_t *then = sema_stmt(bodyInner, stmt->then);
    hlir_t *other = stmt->other == NULL ? NULL : sema_stmt(otherInner, stmt->other);

    return hlir_branch(stmt->node, cond, then, other);
}

static hlir_t *sema_assign(sema_t *sema, ast_t *stmt)
{
    hlir_t *dst = sema_lvalue(sema, stmt->dst);
    hlir_t *src = sema_rvalue(sema, stmt->src);

    hlir_t *convert = convert_to(sema_reports(sema), get_hlir_type(dst), src);

    if (hlir_is(convert, eHlirError))
    {
        report(sema_reports(sema), eFatal, stmt->node, "cannot assign value of type %s to %s",
            ctu_type_repr(sema_reports(sema), get_hlir_type(src), true),
            ctu_repr(sema_reports(sema), dst, true)
        );

        return hlir_error(stmt->node, "invalid assignment");
    }

    return hlir_assign(stmt->node, dst, convert);
}

static hlir_t *sema_stmt(sema_t *sema, ast_t *stmt)
{
    switch (stmt->of)
    {
    case eAstReturn:
        return sema_return(sema, stmt);

    case eAstStmts:
        return sema_stmts(sema, stmt);

    case eAstWhile:
        return sema_while(sema, stmt);

    case eAstDigit:
    case eAstBool:
    case eAstName:
    case eAstString:
    case eAstUnary:
    case eAstBinary:
    case eAstCompare:
    case eAstCall:
        return sema_expr(sema, stmt);

    case eAstVariable:
        return sema_local(sema, stmt);

    case eAstBreak:
        return sema_break(sema, stmt);

    case eAstContinue:
        return sema_continue(sema, stmt);

    case eAstBranch:
        return sema_branch(sema, stmt);

    case eAstAssign:
        return sema_assign(sema, stmt);

    default:
        report(sema_reports(sema), eInternal, stmt->node, "unknown sema-stmt: %d", stmt->of);
        return hlir_error(stmt->node, "unknown sema-stmt");
    }
}

static void check_duplicates_and_add_fields(sema_t *sema, vector_t *fields, hlir_t *decl)
{
    size_t len = vector_len(fields);
    map_t *names = map_optimal(len);

    for (size_t i = 0; i < len; i++)
    {
        ast_t *field = vector_get(fields, i);
        const char *name = field->name;

        if (!is_discard_ident(name))
        {
            const ast_t *previous = map_get(names, name);
            if (previous != NULL)
            {
                report_shadow(sema_reports(sema), name, previous->node, field->node);
                continue;
            }

            map_set(names, name, field);
        }

        if (field->field == NULL)
        {
            continue;
        }

        hlir_t *type = sema_type(sema, field->field);
        hlir_t *entry = hlir_field(field->node, type, name);
        hlir_add_field(decl, entry);
    }
}

static void sema_struct(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    vector_t *fields = ast->fields;

    check_duplicates_and_add_fields(sema, fields, decl);
}

static void sema_union(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    vector_t *fields = ast->fields;

    check_duplicates_and_add_fields(sema, fields, decl);
}

static void check_explicit_tag(sema_t *sema, node_t *node, const hlir_t *base, vector_t *fields)
{
    if (!hlir_is(base, eHlirDigit))
    {
        message_t *id = report(sema_reports(sema), eFatal, node, "tag type must be an integral type");
        report_note(id, "tag type is %s", ctu_type_repr(sema_reports(sema), base, true));
        return;
    }

    size_t len = vector_len(fields);
    for (size_t i = 0; i < len; i++)
    {
        ast_t *field = vector_get(fields, i);
        if (field->value == NULL)
        {
            continue;
        }

        hlir_t *value = sema_rvalue(sema, field->value);
        const hlir_t *ty = hlir_follow_type(get_hlir_type(value));
        if (!hlir_is(ty, eHlirDigit))
        {
            // TODO: also needs to fit in the tag type
            report(sema_reports(sema), eFatal, get_hlir_node(value), "tag value must be an integral type");
            return;
        }

        const hlir_t *common = get_common_type(get_hlir_node(value), base, ty);
        if (hlir_is(common, eHlirError))
        {
            report(sema_reports(sema), eFatal, get_hlir_node(value), "tag value must be of type %s", ctu_type_repr(sema_reports(sema), base, true));
            return;
        }
    }
}

static const hlir_t *get_implicit_tag(sema_t *sema, vector_t *fields)
{
    const hlir_t *base = get_digit_type(eSigned, eDigitChar);

    size_t len = vector_len(fields);
    for (size_t i = 0; i < len; i++)
    {
        ast_t *field = vector_get(fields, i);
        if (field->value == NULL)
        {
            continue;
        }

        hlir_t *value = sema_rvalue(sema, field->value);
        const hlir_t *ty = hlir_follow_type(get_hlir_type(value));
        if (!hlir_is(ty, eHlirDigit))
        {
            report(sema_reports(sema), eFatal, get_hlir_node(value), "tag value must be an integer");
            return NULL;
        }

        base = get_common_type(get_hlir_node(value), base, ty);
    }

    return base;
}

static const hlir_t *get_tag_type(sema_t *sema, ast_t *ast)
{
    if (ast->underlying != NULL)
    {
        const hlir_t *underlying = sema_type(sema, ast->underlying);
        check_explicit_tag(sema, ast->underlying->node, hlir_follow_type(underlying), ast->fields);
        return underlying;
    }
    else
    {
        return get_implicit_tag(sema, ast->fields);
    }
}

static void create_variant_sema(sema_t *parent, const hlir_t *type, ast_t *ast)
{
    size_t len = vector_len(ast->fields);
    size_t decls[eTagTotal] = {
        [eSemaValues] = len
    };

    sema_t *sema = sema_new_checked(parent, eTagTotal, decls);
    for (size_t i = 0; i < len; i++)
    {
        ast_t *field = vector_get(ast->fields, i);
        if (field->value == NULL) { continue; } // TODO: handle generating the value
        hlir_t *value = sema_rvalue(sema, field->value);
        hlir_t *global = hlir_global(field->node, field->name, type, value);
        hlir_set_attributes(global, hlir_attributes(eLinkInternal, eVisiblePublic, eQualConst, NULL));
        sema_set(sema, eSemaValues, field->name, global);
    }

    sema_set(parent, eSemaModules, ast->name, sema);
}

/**
 * variants are internally represented as
 * enum tag_type : underlying { ... }
 * struct {
 *   tag_type tag
 *   union {
 *      fields...
 *   }
 * }
 */
static void sema_variant(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    const hlir_t *tag = get_tag_type(sema, ast);
    // build the tag
    {
        // create the field container for the tag
        hlir_t *field = hlir_field(ast->node, tag, "tag");
        // add the field to the struct
        hlir_add_field(decl, field);
    }

    // build the data
    {
        // create the variant data holder
        char *unionName = format("%s_data", ast->name);
        hlir_t *innerUnion = hlir_begin_union(ast->node, unionName);

        // add all fields with data to the union
        check_duplicates_and_add_fields(sema, ast->fields, innerUnion);

        // create the field container for the union
        hlir_t *dataField = hlir_field(ast->node, innerUnion, "data");

        // add the field to the struct
        hlir_add_field(decl, dataField);
    }

    create_variant_sema(sema, tag, ast);
}

static void sema_params(sema_t *sema, vector_t *params)
{
    size_t len = vector_len(params);

    for (size_t i = 0; i < len; i++)
    {
        hlir_t *param = vector_get(params, i);
        add_decl(sema, eSemaValues, get_hlir_name(param), param);
    }
}

static void update_func_attribs(ast_t *ast, hlir_t *hlir)
{
    linkage_t link = eLinkInternal;
    visibility_t visibility = eVisiblePrivate;
    if (ast->exported)
    {
        link = eLinkExported;
        visibility = eVisiblePublic;
    }
    else if (ast->body == NULL)
    {
        link = eLinkImported;
    }

    hlir_set_attributes(hlir, hlir_attributes(link, visibility, DEFAULT_TAGS, NULL));
}

static void sema_func(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    size_t tags[eTagTotal] = {
        [eSemaValues] = 32,
        [eSemaProcs] = 32,
        [eSemaTypes] = 32,
        [eSemaModules] = 32,
        [eTagAttribs] = 32,
    };

    sema_t *nest = begin_sema(sema, tags);
    set_current_function(nest, decl);
    sema_params(nest, decl->params);

    hlir_t *body = (ast->body != NULL) ? sema_stmts(nest, ast->body) : NULL;

    hlir_build_function(decl, body);
    update_func_attribs(ast, decl);
}

static void sema_decl(sema_t *sema, ast_t *ast)
{
    hlir_t *decl;

    switch (ast->of)
    {
    case eAstDeclStruct:
        decl = sema_get_resolved(sema, eSemaTypes, ast->name);
        sema_struct(sema, decl, ast);
        break;

    case eAstDeclUnion:
        decl = sema_get_resolved(sema, eSemaTypes, ast->name);
        sema_union(sema, decl, ast);
        break;

    case eAstDeclAlias:
        decl = sema_get_resolved(sema, eSemaTypes, ast->name);
        break;

    case eAstDeclVariant:
        decl = sema_get_resolved(sema, eSemaTypes, ast->name);
        sema_variant(sema, decl, ast);
        break;

    case eAstFunction:
        decl = sema_get_resolved(sema, eSemaProcs, ast->name);
        sema_func(sema, decl, ast);
        break;

    case eAstVariable:
        decl = sema_get_resolved(sema, eSemaValues, ast->name);
        break;

    default:
        ctu_assert(sema_reports(sema), "unexpected ast of type %d", ast->of);
        return;
    }

    apply_attributes(sema, decl, ast);
}

static arity_t get_arity(ast_t *ast)
{
    CTASSERT(ast->of == eAstFunction);
    ast_t *sig = ast->signature;

    if (vector_len(sig->params) == 0)
    {
        return eArityFixed;
    }

    ast_t *tail = vector_tail(sig->params);
    CTASSERT(tail->of == eAstParam);

    ast_t *type = tail->param;
    return (type->of == eAstVarArgs) ? eArityVariable : eArityFixed;
}

static hlir_t *get_func_result(sema_t *sema, ast_t *result)
{
    if (result == NULL)
    {
        return kVoidType;
    }
    else if (result->of == eAstVarArgs)
    {
        report(sema_reports(sema), eFatal, result->node, "function cannot have a single varargs parameter");
        return kVoidType;
    }

    return sema_type(sema, result);
}

static hlir_t *begin_function(sema_t *sema, ast_t *ast)
{
    ast_t *signature = ast->signature;
    hlir_t *result = get_func_result(sema, signature->result);

    size_t len = vector_len(signature->params);

    arity_t arity = get_arity(ast);

    if (len == 1 && arity == eArityVariable)
    {
        report(sema_reports(sema), eFatal, ast->node, "function cannot have a single varargs parameter");
    }

    if (arity == eArityVariable)
    {
        len -= 1;
    }

    vector_t *params = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        ast_t *param = vector_get(signature->params, i);

        const char *name = param->name;
        ast_t *type = param->param;

        const hlir_t *hlir = sema_type(sema, type);
        hlir_t *entry = hlir_param(param->node, name, hlir);

        vector_set(params, i, entry);
    }

    hlir_t *func = hlir_begin_function(ast->node, ast->name, params, result, arity);
    update_func_attribs(ast, func);
    return func;
}

static void fwd_decl(sema_t *sema, ast_t *ast)
{
    int tag = eSemaTypes;

    switch (ast->of)
    {
    case eAstDeclStruct:
    case eAstDeclUnion:
    case eAstDeclAlias:
    case eAstDeclVariant:
        break;

    case eAstFunction:
        tag = eSemaProcs;
        break;

    case eAstVariable:
        tag = eSemaValues;
        break;

    default:
        ctu_assert(sema_reports(sema), "unexpected ast of type %d", ast->of);
        return;
    }

    hlir_t *decl = hlir_unresolved(ast->node, ast->name, sema, ast);
    hlir_set_attributes(decl, hlir_attributes(eLinkExported, ast->exported ? eVisiblePublic : eVisiblePrivate, DEFAULT_TAGS, NULL));

    add_decl(sema, tag, ast->name, decl);
}

static char *make_import_name(vector_t *vec)
{
    return str_join(".", vec);
}

static void import_namespaced_decls(sema_t *sema, ast_t *import, sema_t *mod)
{
    const char *name = vector_tail(import->path);
    sema_t *previous = sema_get(sema, eSemaModules, name);

    if (previous != NULL)
    {
        message_t *id =
            report(sema_reports(sema), eFatal, import->node, "a module was already imported under the name `%s`", name);
        report_note(id, "use module aliases to avoid name collisions");
        return;
    }

    sema_set(sema, eSemaModules, name, mod);
}

void ctu_forward_decls(runtime_t *runtime, compile_t *compile)
{
    UNUSED(runtime);
    ast_t *root = compile->ast;

    size_t totalDecls = vector_len(root->decls);
    size_t sizes[eTagTotal] = {
        [eSemaValues] = totalDecls,  [eSemaProcs] = totalDecls,
        [eSemaTypes] = totalDecls,   [eSemaModules] = vector_len(root->imports),
        [eTagAttribs] = totalDecls, [eTagSuffix] = 32,
    };

    sema_t *sema = begin_sema(kRootSema, sizes);
    add_builtin_attribs(sema);

    if (root->modspec != NULL)
    {
        compile->moduleName = make_import_name(root->modspec->path);
    }

    hlir_t *mod = hlir_module(root->node, compile->moduleName, vector_of(0), vector_of(0), vector_of(0));

    sema_data_t semaData = {.totalDecls = totalDecls};

    sema_set_data(sema, BOX(semaData));

    for (size_t i = 0; i < totalDecls; i++)
    {
        ast_t *decl = vector_get(root->decls, i);
        fwd_decl(sema, decl);
    }

    compile->sema = sema;
    compile->hlir = mod;
}

void ctu_process_imports(runtime_t *runtime, compile_t *compile)
{
    ast_t *root = compile->ast;
    sema_t *sema = compile->sema;

    vector_t *imports = root->imports;
    size_t totalImports = vector_len(imports);

    for (size_t i = 0; i < totalImports; i++)
    {
        ast_t *import = vector_get(imports, i);
        char *name = make_import_name(import->path);

        sema_t *mod = find_module(runtime, name);
        if (mod == NULL)
        {
            report(runtime->reports, eFatal, import->node, "module '%s' not found", name);
            continue;
        }

        if (mod == sema)
        {
            report(runtime->reports, eWarn, import->node, "module cannot import itself");
            continue;
        }

        import_namespaced_decls(sema, import, mod);
    }
}

hlir_t *ctu_compile_module(runtime_t *runtime, compile_t *compile)
{
    UNUSED(runtime);

    ast_t *root = compile->ast;
    sema_t *sema = compile->sema;
    sema_data_t *data = sema_get_data(sema);

    for (size_t i = 0; i < data->totalDecls; i++)
    {
        ast_t *decl = vector_get(root->decls, i);
        sema_decl(sema, decl);
    }

    vector_t *types = map_values(sema_tag(sema, eSemaTypes));
    vector_t *globals = map_values(sema_tag(sema, eSemaValues));
    vector_t *procs = map_values(sema_tag(sema, eSemaProcs));

    hlir_build_module(compile->hlir, types, globals, procs);

    return compile->hlir;
}
