#include "sema.h"
#include "ast.h"
#include "attribs.h"
#include "cthulhu/hlir/attribs.h"
#include "repr.h"

#include "cthulhu/hlir/decl.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/type.h"
#include "cthulhu/interface/runtime.h"

#include "base/macros.h"
#include "base/util.h"
#include "base/memory.h"

#include "report/report-ext.h"
#include "std/set.h"
#include "std/str.h"

typedef struct
{
    size_t totalDecls;
    hlir_t *parentModule;

    hlir_t *currentFunction; // current function we are adding locals to
} sema_data_t;

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

static sema_t *begin_sema(sema_t *parent, reports_t *reports, size_t *sizes)
{
    sema_data_t *data = parent != NULL ? sema_get_data(parent) : ctu_malloc(sizeof(sema_data_t));
    if (parent == NULL)
    {
        data->currentFunction = NULL;
    }

    sema_t *sema = sema_new(parent, (reports != NULL ? reports : parent->reports), eTagTotal, sizes);
    sema_set_data(sema, data);
    return sema;
}

static const char *kDigitNames[eSignTotal][eDigitTotal] = {
    [eSigned] =
        {
            [eChar] = "char",
            [eShort] = "short",
            [eInt] = "int",
            [eLong] = "long",

            [eIntPtr] = "intptr",
            [eIntSize] = "size",
            [eIntMax] = "intmax",
        },
    [eUnsigned] =
        {
            [eChar] = "uchar",
            [eShort] = "ushort",
            [eInt] = "uint",
            [eLong] = "ulong",

            [eIntPtr] = "uintptr",
            [eIntSize] = "usize",
            [eIntMax] = "uintmax",
        },
};

static hlir_t *kVoidType = NULL;
static hlir_t *kBoolType = NULL;
static hlir_t *kStringType = NULL;
static hlir_t *kDigitTypes[eSignTotal * eDigitTotal];

static hlir_t *kUnresolvedType = NULL;
static hlir_t *kUninitalized = NULL;

#define DIGIT_INDEX(sign, digit) ((sign)*eDigitTotal + (digit))

static hlir_t *get_digit_type(sign_t sign, digit_t digit)
{
    return kDigitTypes[DIGIT_INDEX(sign, digit)];
}

static const char *get_digit_name(sign_t sign, digit_t digit)
{
    return kDigitNames[sign][digit];
}

static sema_t *kRootSema = NULL;

static const hlir_t *get_common_type(node_t node, const hlir_t *lhs, const hlir_t *rhs)
{
    if (hlir_is(lhs, eHlirDigit) && hlir_is(rhs, eHlirDigit))
    {
        // get the largest size
        digit_t width = MAX(lhs->width, rhs->width);
        // if either is signed the result is signed
        sign_t sign = (lhs->sign == eSigned || rhs->sign == eSigned) ? eSigned : eUnsigned;

        return get_digit_type(sign, width);
    }

    return hlir_error(node, "unknown common type");
}

static bool is_discard_ident(const char *id)
{
    return id == NULL || str_equal(id, "$");
}

static void add_decl(sema_t *sema, tag_t tag, const char *name, hlir_t *decl)
{
    node_t node = get_hlir_node(decl);

    if (is_discard_ident(name))
    {
        report(sema->reports, eFatal, node, "discarding declaration");
        return;
    }

    // was this already declared?
    hlir_t *other = sema_get(sema, tag, name);
    if (other != NULL)
    {
        // if it was report it and dont add this new one
        node_t otherNode = get_hlir_node(other);
        report_shadow(sema->reports, name, otherNode, node);
        return;
    }

    sema_set(sema, tag, name, decl);
}

static void add_basic_types(sema_t *sema)
{
    add_decl(sema, eTagTypes, "void", kVoidType);
    add_decl(sema, eTagTypes, "bool", kBoolType);
    add_decl(sema, eTagTypes, "str", kStringType);

    for (int sign = 0; sign < eSignTotal; sign++)
    {
        for (int digit = 0; digit < eDigitTotal; digit++)
        {
            const char *name = get_digit_name(sign, digit);
            hlir_t *type = get_digit_type(sign, digit);

            add_decl(sema, eTagTypes, name, type);
        }
    }

    // enable the below later

    // noreturn type for exit/abort/terminate etc
    // add_decl(sema, eTagTypes, "noreturn", hlir_noreturn(node, "noreturn"));

    // special types for interfacing with C
    // add_decl(sema, eTagTypes, "enum", hlir_digit(node, "enum", eInt, eSigned));
}

void ctu_init_compiler(runtime_t *runtime)
{
    size_t sizes[eTagTotal] = {
        [eTagValues] = 1, [eTagProcs] = 1, [eTagTypes] = 32, [eTagModules] = 1, [eTagAttribs] = 1};

    kRootSema = begin_sema(NULL, runtime->reports, sizes);

    node_t node = node_builtin();

    kVoidType = hlir_void(node, "void");
    kBoolType = hlir_bool(node, "bool");
    kStringType = hlir_string(node, "str");

    for (int sign = 0; sign < eSignTotal; sign++)
    {
        for (int digit = 0; digit < eDigitTotal; digit++)
        {
            const char *name = get_digit_name(sign, digit);
            kDigitTypes[DIGIT_INDEX(sign, digit)] = hlir_digit(node, name, digit, sign);
        }
    }

    add_basic_types(kRootSema);

    kUnresolvedType = hlir_error(node, "unresolved");
    kUninitalized = hlir_error(node, "uninitialized");
}

static hlir_t *sema_type(sema_t *sema, ast_t *ast);

static hlir_t *sema_expr(sema_t *sema, ast_t *ast);

static hlir_t *sema_typename(sema_t *sema, ast_t *ast)
{
    size_t len = vector_len(ast->path);
    sema_t *current = sema;
    for (size_t i = 0; i < len - 1; i++)
    {
        const char *name = vector_get(ast->path, i);
        sema_t *next = sema_get(current, eTagModules, name);

        if (next == NULL)
        {
            report(sema->reports, eFatal, ast->node, "unknown namespace `%s`", name);
            return hlir_error(ast->node, "unknown namespace");
        }

        current = next;
    }

    const char *name = vector_tail(ast->path);
    while (current != NULL)
    {
        hlir_t *decl = sema_get(current, eTagTypes, name);
        if (decl != NULL)
        {
            return decl;
        }

        current = current->parent;
    }

    report(sema->reports, eFatal, ast->node, "type '%s' not found", name);
    return hlir_error(ast->node, "type not found");
}

static hlir_t *sema_pointer(sema_t *sema, ast_t *ast)
{
    hlir_t *type = sema_type(sema, ast->type);
    return hlir_pointer(ast->node, NULL, type, ast->indexable);
}

static hlir_t *sema_array(sema_t *sema, ast_t *ast)
{
    hlir_t *size = sema_expr(sema, ast->size);
    hlir_t *type = sema_type(sema, ast->type);

    return hlir_array(sema->reports, ast->node, NULL, type, size);
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

        if (hlir_is(type, eHlirVoid))
        {
            report(sema->reports, eFatal, param->node, "void parameter");
        }
    }

    return hlir_closure(ast->node, NULL, params, result, ast->variadic);
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
        report(sema->reports, eInternal, ast->node, "unknown sema-type: %d", ast->of);
        return hlir_error(ast->node, "unknown sema-type");
    }
}

static hlir_t *sema_digit(ast_t *ast)
{
    // TODO: check if the digit is in range
    // TODO: digit suffixes should be added later
    // TODO: or maybe we want untyped literals?

    const hlir_t *type = get_digit_type(eSigned, eInt);
    return hlir_digit_literal(ast->node, type, ast->digit);
}

static hlir_t *sema_bool(ast_t *ast)
{
    return hlir_bool_literal(ast->node, kBoolType, ast->boolean);
}

static hlir_t *sema_string(ast_t *ast)
{
    return hlir_string_literal(ast->node, kStringType, ast->string, ast->length);
}

static hlir_t *sema_unary_digit(sema_t *sema, ast_t *ast, hlir_t *operand)
{
    const hlir_t *type = get_hlir_type(operand);
    const hlir_t *realType = hlir_real_type(type);

    if (!hlir_is(realType, eHlirDigit))
    {
        report(sema->reports, eFatal, ast->node, "cannot perform integer unary operation on '%s'",
               ctu_repr(sema->reports, operand, true));
        return hlir_error(ast->node, "invalid unary digit type");
    }

    return hlir_unary(ast->node, type, operand, ast->unary);
}

static hlir_t *sema_unary_bool(sema_t *sema, ast_t *ast, hlir_t *operand)
{
    const hlir_t *type = get_hlir_type(operand);
    const hlir_t *realType = hlir_real_type(type);

    if (!hlir_is(realType, eHlirBool))
    {
        report(sema->reports, eFatal, ast->node, "cannot perform boolean unary operation on '%s'",
               ctu_repr(sema->reports, operand, true));
        return hlir_error(ast->node, "invalid unary bool type");
    }

    return hlir_unary(ast->node, type, operand, ast->unary);
}

static hlir_t *sema_unary(sema_t *sema, ast_t *ast)
{
    hlir_t *operand = sema_expr(sema, ast->operand);

    switch (ast->unary)
    {
    case eUnaryAbs:
    case eUnaryNeg:
    case eUnaryBitflip:
        return sema_unary_digit(sema, ast, operand);

    case eUnaryNot:
        return sema_unary_bool(sema, ast, operand);

    default:
        report(sema->reports, eInternal, ast->node, "unexpected unary operand %s", unary_name(ast->unary));
        return hlir_error(ast->node, "unsupported unary operand");
    }
}

static hlir_t *sema_binary(sema_t *sema, ast_t *ast)
{
    hlir_t *lhs = sema_expr(sema, ast->lhs);
    hlir_t *rhs = sema_expr(sema, ast->rhs);

    const hlir_t *type = get_common_type(ast->node, get_hlir_type(lhs), get_hlir_type(rhs));

    if (!hlir_is(type, eHlirDigit))
    {
        message_t *id = report(sema->reports, eFatal, ast->node, "cannot perform binary operations on %s",
                               ctu_type_repr(sema->reports, type, true));
        report_append(id, get_hlir_node(lhs), ctu_repr(sema->reports, lhs, false));
        report_append(id, get_hlir_node(rhs), ctu_repr(sema->reports, rhs, false));
        return hlir_error(ast->node, "invalid common type");
    }

    return hlir_binary(ast->node, type, ast->binary, lhs, rhs);
}

static hlir_t *sema_compare(sema_t *sema, ast_t *ast)
{
    hlir_t *lhs = sema_expr(sema, ast->lhs);
    hlir_t *rhs = sema_expr(sema, ast->rhs);

    const hlir_t *type = get_common_type(ast->node, get_hlir_type(lhs), get_hlir_type(rhs));

    if (!hlir_is(type, eHlirDigit))
    {
        message_t *id = report(sema->reports, eFatal, ast->node, "cannot perform comparison operations on %s",
                               ctu_type_repr(sema->reports, type, true));
        report_append(id, get_hlir_node(lhs), ctu_repr(sema->reports, lhs, false));
        report_append(id, get_hlir_node(rhs), ctu_repr(sema->reports, rhs, false));
        return hlir_error(ast->node, "invalid common type");
    }

    return hlir_compare(ast->node, kBoolType, ast->compare, lhs, rhs);
}

void check_valid_import(sema_t *sema, sema_t *cur, ast_t *ast, hlir_t *hlir)
{
    const char *name = vector_tail(ast->path);
    if (sema != cur && !hlir_is_exported(hlir))
    {
        message_t *id =
            report(sema->reports, eFatal, ast->node, "symbol '%s' is not visible inside this context", name);
        report_append(id, get_hlir_node(hlir), "originally declared here");
    }
}

static hlir_t *sema_ident(sema_t *sema, ast_t *ast)
{
    sema_t *current = sema;
    for (size_t i = 0; i < vector_len(ast->path) - 1; i++)
    {
        const char *name = vector_get(ast->path, i);
        if (name == NULL)
        {
            report(sema->reports, eFatal, ast->node, "discarded path segment");
            return hlir_error(ast->node, "discarded path segment");
        }

        sema_t *next = sema_get(current, eTagModules, name);

        if (next == NULL)
        {
            report(sema->reports, eFatal, ast->node, "unknown namespace `%s`", name);
            return hlir_error(ast->node, "unknown namespace");
        }

        current = next;
    }

    const char *name = vector_tail(ast->path);

    if (name == NULL)
    {
        report(sema->reports, eFatal, ast->node, "cannot resolve discarded identifier");
        return hlir_error(ast->node, "discarded indentifier");
    }

    hlir_t *var = sema_get(current, eTagValues, name);
    if (var != NULL)
    {
        check_valid_import(sema, current, ast, var);
        return hlir_name(ast->node, var);
    }

    hlir_t *func = sema_get(sema, eTagProcs, name);
    if (func != NULL)
    {
        check_valid_import(sema, current, ast, func);
        return func;
    }

    report(sema->reports, eFatal, ast->node, "unknown identifier '%s'", name);
    return hlir_error(ast->node, "unknown identifier");
}

static hlir_t *sema_call(sema_t *sema, ast_t *ast)
{
    hlir_t *call = sema_expr(sema, ast->call);
    if (!hlis_is_or_will_be(call, eHlirFunction))
    {
        message_t *id = report(sema->reports, eFatal, ast->node, "can only call function types");
        report_underline(id, "%s", ctu_repr(sema->reports, call, true));
        return hlir_error(ast->node, "invalid callable");
    }

    size_t len = vector_len(ast->args);
    vector_t *params = closure_params(call);
    size_t totalParams = vector_len(params);
    if (len != totalParams)
    {
        message_t *id = report(sema->reports, eFatal, ast->node, "incorrect number of parameters specified");
        report_note(id, "expected `%zu` got `%zu` instead", totalParams, len);
        return hlir_error(ast->node, "incorrect argument count");
    }

    vector_t *args = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        ast_t *arg = vector_get(ast->args, i);
        hlir_t *hlir = sema_expr(sema, arg);
        hlir_t *expectedType = vector_get(params, i);
        const hlir_t *argType = get_hlir_type(hlir);

        if (!hlir_types_equal(argType, expectedType))
        {
            message_t *id = report(sema->reports, eFatal, arg->node, "incorrect argument type `%s`",
                                   ctu_repr(sema->reports, hlir, true));
            report_note(id, "expecting '%s' instead", ctu_type_repr(sema->reports, expectedType, true));
        }

        vector_set(args, i, hlir);
    }

    return hlir_call(ast->node, call, args);
}

static hlir_t *sema_expr(sema_t *sema, ast_t *ast)
{
    switch (ast->of)
    {
    case eAstDigit:
        return sema_digit(ast);
    case eAstBool:
        return sema_bool(ast);
    case eAstString:
        return sema_string(ast);
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

    default:
        report(sema->reports, eInternal, ast->node, "unknown sema-expr: %d", ast->of);
        return hlir_error(ast->node, "unknown sema-expr");
    }
}

static hlir_t *sema_stmt(sema_t *sema, ast_t *stmt);

static hlir_t *sema_stmts(sema_t *sema, ast_t *stmts)
{
    size_t len = vector_len(stmts->stmts);

    vector_t *result = vector_of(len);

    for (size_t i = 0; i < len; i++)
    {
        ast_t *stmt = vector_get(stmts->stmts, i);
        hlir_t *hlir = sema_stmt(sema, stmt);
        vector_set(result, i, hlir);
    }

    return hlir_stmts(stmts->node, result);
}

static hlir_t *sema_return(sema_t *sema, ast_t *ast)
{
    hlir_t *result = sema_expr(sema, ast->operand);

    return hlir_return(ast->node, result);
}

static hlir_t *sema_while(sema_t *sema, ast_t *ast)
{
    if (ast->label != NULL)
    {
        report(sema->reports, eInternal, ast->node, "loop labels not yet supported");
    }

    hlir_t *cond = sema_expr(sema, ast->cond);

    size_t sizes[eTagTotal] = {
        [eTagValues] = 32, [eTagProcs] = 32, [eTagTypes] = 32, [eTagModules] = 1, [eTagAttribs] = 1};

    sema_t *nestThen = begin_sema(sema, sema->reports, sizes);

    hlir_t *then = sema_stmt(nestThen, ast->then);
    hlir_t *other = NULL;

    if (ast->other != NULL)
    {
        sema_t *nextOther = begin_sema(sema, sema->reports, sizes);
        other = sema_stmt(nextOther, ast->other);
    }

    const hlir_t *condType = get_hlir_type(cond);
    if (!hlir_types_equal(condType, kBoolType))
    {
        message_t *id = report(sema->reports, eFatal, get_hlir_node(cond), "loop condition must be boolean");
        report_note(id, "type '%s' found", ctu_repr(sema->reports, cond, true));
    }

    return hlir_loop(ast->node, cond, then, other);
}

typedef struct
{
    const hlir_t *type;
    hlir_t *init;
} sema_value_t;

static sema_value_t sema_value(sema_t *sema, ast_t *stmt)
{
    hlir_t *init = stmt->init != NULL ? sema_expr(sema, stmt->init) : NULL;
    const hlir_t *type = stmt->expected != NULL ? sema_type(sema, stmt->expected) : get_hlir_type(init);

    sema_value_t result = {type, init};

    if ((stmt->init != NULL && stmt->expected != NULL) && !hlir_types_equal(type, get_hlir_type(init)))
    {
        message_t *id = report(sema->reports, eFatal, stmt->node, "incompatible initializer and explicit type");
        report_underline(id, "found '%s', expected '%s'", ctu_type_repr(sema->reports, type, true),
                         ctu_repr(sema->reports, init, true));
        result.init = hlir_error(stmt->node, "invalid value declaration");
        return result;
    }

    if (!stmt->mut)
    {
        hlir_t *constant = ctu_memdup(type, sizeof(hlir_t));
        hlir_set_attributes(constant, hlir_tags(eTagConst));
        result.type = constant;
    }

    return result;
}

// TODO: merge this logic with global init
static hlir_t *sema_local(sema_t *sema, ast_t *stmt)
{
    sema_value_t value = sema_value(sema, stmt);

    if (value.init != NULL && hlir_is(value.init, eHlirError))
    {
        return value.init;
    }

    hlir_t *local = hlir_local(stmt->node, stmt->name, value.type);
    add_decl(sema, eTagValues, stmt->name, local);
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
        report(sema->reports, eInternal, stmt->node, "loop labels not yet supported"); // TODO: support labels
    }

    return hlir_break(stmt->node, NULL);
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

    default:
        report(sema->reports, eInternal, stmt->node, "unknown sema-stmt: %d", stmt->of);
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
                report_shadow(sema->reports, name, previous->node, field->node);
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

    hlir_build_struct(decl);
}

static void sema_union(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    vector_t *fields = ast->fields;

    check_duplicates_and_add_fields(sema, fields, decl);

    hlir_build_union(decl);
}

static void sema_alias(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    hlir_t *type = sema_type(sema, ast->alias);
    hlir_build_alias(decl, type, ast->newtype);
}

/**
 * variants are internally represented as
 * struct {
 *   tag_type tag;
 *   union {
 *      fields...
 *   }
 * }
 */
static void sema_variant(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    // build the tag
    {
        // create the variant tag
        char *tagName = format("%s_tag", ast->name);
        hlir_t *tag = hlir_digit(ast->node, tagName, eInt, eUnsigned);

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

        // then build the union and set its parent
        hlir_build_union(innerUnion);
        hlir_set_parent(innerUnion, decl);

        // create the field container for the union
        hlir_t *dataField = hlir_field(ast->node, innerUnion, "data");

        // add the field to the struct
        hlir_add_field(decl, dataField);
    }

    // add the variant to the struct
    hlir_build_struct(decl);
}

static void sema_params(sema_t *sema, vector_t *params)
{
    size_t len = vector_len(params);

    for (size_t i = 0; i < len; i++)
    {
        hlir_t *param = vector_get(params, i);
        add_decl(sema, eTagValues, get_hlir_name(param), param);
    }
}

static void sema_func(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    hlir_attributes_t *attribs = hlir_attributes(ast->body == NULL ? eLinkImported : eLinkExported, 0, NULL);

    size_t tags[eTagTotal] = {
        [eTagValues] = 32, [eTagProcs] = 32, [eTagTypes] = 32, [eTagModules] = 32, [eTagAttribs] = 32};

    sema_t *nest = begin_sema(sema, sema->reports, tags);
    set_current_function(nest, decl);
    sema_params(nest, decl->params);

    hlir_t *body = (ast->body != NULL) ? sema_stmts(nest, ast->body) : NULL;

    hlir_build_function(decl, body);
    hlir_set_attributes(decl, attribs);
}

static void sema_global(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    sema_value_t result = sema_value(sema, ast);

    if (result.init != NULL && hlir_is(result.init, eHlirError))
    {
        return;
    }

    hlir_set_type(decl, result.type);
    hlir_build_global(decl, result.init);
}

static void sema_decl(sema_t *sema, ast_t *ast)
{
    hlir_t *decl;

    switch (ast->of)
    {
    case eAstDeclStruct:
        decl = sema_get(sema, eTagTypes, ast->name);
        sema_struct(sema, decl, ast);
        break;

    case eAstDeclUnion:
        decl = sema_get(sema, eTagTypes, ast->name);
        sema_union(sema, decl, ast);
        break;

    case eAstDeclAlias:
        decl = sema_get(sema, eTagTypes, ast->name);
        sema_alias(sema, decl, ast);
        break;

    case eAstDeclVariant:
        decl = sema_get(sema, eTagTypes, ast->name);
        sema_variant(sema, decl, ast);
        break;

    case eAstFunction:
        decl = sema_get(sema, eTagProcs, ast->name);
        sema_func(sema, decl, ast);
        break;

    case eAstVariable:
        decl = sema_get(sema, eTagValues, ast->name);
        sema_global(sema, decl, ast);
        break;

    default:
        ctu_assert(sema->reports, "unexpected ast of type %d", ast->of);
        return;
    }

    if (ast->exported)
    {
        hlir_set_attributes(decl, hlir_linkage(eLinkExported));
    }

    apply_attributes(sema, decl, ast);
}

static hlir_t *begin_function(sema_t *sema, ast_t *ast)
{
    hlir_t *result = kVoidType;
    ast_t *signature = ast->signature;
    if (signature->result != NULL)
    {
        result = sema_type(sema, signature->result);
    }

    size_t len = vector_len(signature->params);
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

    signature_t sig = {.params = params, .result = result, .variadic = false};

    return hlir_begin_function(ast->node, ast->name, sig);
}

static hlir_t *begin_global(ast_t *ast)
{
    return hlir_begin_global(ast->node, ast->name, kUnresolvedType);
}

static void fwd_decl(sema_t *sema, ast_t *ast)
{
    hlir_t *decl;
    tag_t tag = eTagTypes;

    switch (ast->of)
    {
    case eAstDeclStruct:
        decl = hlir_begin_struct(ast->node, ast->name);
        break;

    case eAstDeclUnion:
        decl = hlir_begin_union(ast->node, ast->name);
        break;

    case eAstDeclAlias:
        decl = hlir_begin_alias(ast->node, ast->name);
        break;

    case eAstDeclVariant:
        decl = hlir_begin_struct(ast->node, ast->name);
        break;

    case eAstFunction:
        decl = begin_function(sema, ast);
        tag = eTagProcs;
        break;

    case eAstVariable:
        decl = begin_global(ast);
        tag = eTagValues;
        break;

    default:
        ctu_assert(sema->reports, "unexpected ast of type %d", ast->of);
        return;
    }

    sema_data_t *semaData = sema_get_data(sema);

    hlir_set_parent(decl, semaData->parentModule);
    add_decl(sema, tag, ast->name, decl);
}

static char *make_import_name(vector_t *vec)
{
    return str_join(".", vec);
}

static void import_namespaced_decls(sema_t *sema, ast_t *import, sema_t *mod)
{
    const char *name = vector_tail(import->path);
    sema_t *previous = sema_get(sema, eTagModules, name);

    if (previous != NULL)
    {
        message_t *id =
            report(sema->reports, eFatal, import->node, "a module was already imported under the name `%s`", name);
        report_note(id, "use module aliases to avoid name collisions");
        return;
    }

    sema_set(sema, eTagModules, name, mod);
}

void ctu_forward_decls(runtime_t *runtime, compile_t *compile)
{
    ast_t *root = compile->ast;

    size_t totalDecls = vector_len(root->decls);
    size_t sizes[eTagTotal] = {[eTagValues] = totalDecls,
                               [eTagProcs] = totalDecls,
                               [eTagTypes] = totalDecls,
                               [eTagModules] = vector_len(root->imports),
                               [eTagAttribs] = totalDecls};

    sema_t *sema = begin_sema(kRootSema, runtime->reports, sizes);
    add_builtin_attribs(sema);

    char *name = NULL;
    if (root->modspec != NULL)
    {
        name = make_import_name(root->modspec->path);
    }

    hlir_t *mod = hlir_module(root->node, name, vector_of(0), vector_of(0), vector_of(0));

    sema_data_t semaData = {.totalDecls = totalDecls, .parentModule = mod};

    sema_set_data(sema, BOX(semaData));

    for (size_t i = 0; i < totalDecls; i++)
    {
        ast_t *decl = vector_get(root->decls, i);
        fwd_decl(sema, decl);
    }

    vector_t *types = map_values(sema_tag(sema, eTagTypes));
    vector_t *globals = map_values(sema_tag(sema, eTagValues));
    vector_t *procs = map_values(sema_tag(sema, eTagProcs));

    hlir_update_module(mod, types, globals, procs);

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

void ctu_compile_module(runtime_t *runtime, compile_t *compile)
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

    vector_t *types = map_values(sema_tag(sema, eTagTypes));
    vector_t *globals = map_values(sema_tag(sema, eTagValues));
    vector_t *procs = map_values(sema_tag(sema, eTagProcs));

    hlir_update_module(compile->hlir, types, globals, procs);
}
