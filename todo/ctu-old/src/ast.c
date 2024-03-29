#include "ast.h"

#include "base/memory.h"

#include "report/report.h"

#include "sema/config.h"

#include <string.h>

static ast_t *ast_new(astof_t of, scan_t *scan, where_t where)
{
    ast_t *ast = ctu_malloc(sizeof(ast_t));
    ast->of = of;
    ast->node = node_new(scan, where);
    return ast;
}

static ast_t *ast_decl(astof_t of, char *name, scan_t *scan, where_t where)
{
    ast_t *ast = ast_new(of, scan, where);
    ast->name = name;
    ast->attribs = NULL;
    return ast;
}

ast_t *ast_program(scan_t *scan, where_t where, ast_t *modspec, vector_t *imports, vector_t *decls)
{
    ast_t *ast = ast_new(eAstProgram, scan, where);
    ast->modspec = modspec;
    ast->imports = imports;
    ast->decls = decls;
    return ast;
}

ast_t *ast_import(scan_t *scan, where_t where, vector_t *path, char *alias, vector_t *items)
{
    ast_t *ast = ast_new(eAstImport, scan, where);
    ast->path = path;
    ast->items = items;
    ast->id = alias;
    return ast;
}

ast_t *ast_module(scan_t *scan, where_t where, vector_t *path)
{
    ast_t *ast = ast_new(eAstModule, scan, where);
    ast->path = path;
    return ast;
}

ast_t *ast_attribute(scan_t *scan, where_t where, char *name, vector_t *args)
{
    ast_t *ast = ast_decl(eAstAttribute, name, scan, where);
    ast->config = args;
    return ast;
}

ast_t *ast_function(scan_t *scan, where_t where, char *name, ast_t *signature, ast_t *body)
{
    ast_t *ast = ast_decl(eAstFunction, name, scan, where);
    ast->signature = signature;
    ast->body = body;
    return ast;
}

ast_t *ast_variable(scan_t *scan, where_t where, char *name, bool mut, ast_t *expected, ast_t *init)
{
    ast_t *ast = ast_decl(eAstVariable, name, scan, where);
    ast->mut = mut;
    ast->expected = expected;
    ast->init = init;

    if (expected == NULL && init == NULL)
    {
        report(scan_reports(scan), eFatal, ast->node, "uninitialized variable requires an explicit type annotation");
    }

    return ast;
}

ast_t *ast_digit(scan_t *scan, where_t where, mpz_t value, char *suffix)
{
    ast_t *ast = ast_new(eAstDigit, scan, where);
    memcpy(ast->digit, value, sizeof(mpz_t));
    ast->suffix = suffix;
    return ast;
}

ast_t *ast_decimal(scan_t *scan, where_t where, mpq_t value)
{
    ast_t *ast = ast_new(eAstDecimal, scan, where);
    memcpy(ast->decimal, value, sizeof(mpq_t));
    return ast;
}

ast_t *ast_bool(scan_t *scan, where_t where, bool value)
{
    ast_t *ast = ast_new(eAstBool, scan, where);
    ast->boolean = value;
    return ast;
}

ast_t *ast_string(scan_t *scan, where_t where, char *str, size_t length)
{
    ast_t *ast = ast_new(eAstString, scan, where);
    ast->string = str;
    ast->length = length;
    return ast;
}

ast_t *ast_null(scan_t *scan, where_t where)
{
    return ast_new(eAstNull, scan, where);
}

ast_t *ast_name(scan_t *scan, where_t where, vector_t *path)
{
    ast_t *ast = ast_new(eAstName, scan, where);
    ast->path = path;
    return ast;
}

ast_t *ast_unary(scan_t *scan, where_t where, unary_t op, ast_t *operand)
{
    ast_t *ast = ast_new(eAstUnary, scan, where);
    ast->unary = op;
    ast->operand = operand;
    return ast;
}

ast_t *ast_binary(scan_t *scan, where_t where, binary_t binary, ast_t *lhs, ast_t *rhs)
{
    ast_t *ast = ast_new(eAstBinary, scan, where);
    ast->binary = binary;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

ast_t *ast_compare(scan_t *scan, where_t where, compare_t compare, ast_t *lhs, ast_t *rhs)
{
    ast_t *ast = ast_new(eAstCompare, scan, where);
    ast->compare = compare;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

ast_t *ast_access(scan_t *scan, where_t where, ast_t *data, const char *field, bool indirect)
{
    ast_t *ast = ast_new(eAstAccess, scan, where);

    ast->record = data;
    ast->access = field;
    ast->indirect = indirect;

    return ast;
}

ast_t *ast_index(scan_t *scan, where_t where, ast_t *array, ast_t *index)
{
    ast_t *ast = ast_new(eAstIndex, scan, where);
    ast->array = array;
    ast->index = index;
    return ast;
}

ast_t *ast_call(scan_t *scan, where_t where, ast_t *call, vector_t *args)
{
    ast_t *ast = ast_new(eAstCall, scan, where);
    ast->call = call;
    ast->args = args;
    return ast;
}

ast_t *ast_ref(scan_t *scan, where_t where, ast_t *value)
{
    ast_t *ast = ast_new(eAstRef, scan, where);
    ast->operand = value;
    return ast;
}

ast_t *ast_deref(scan_t *scan, where_t where, ast_t *value)
{
    ast_t *ast = ast_new(eAstDeref, scan, where);
    ast->operand = value;
    return ast;
}

ast_t *ast_sizeof(scan_t *scan, where_t where, ast_t *type)
{
    ast_t *ast = ast_new(eAstSizeOf, scan, where);
    ast->type = type;
    return ast;
}

ast_t *ast_stmts(scan_t *scan, where_t where, vector_t *stmts)
{
    ast_t *ast = ast_new(eAstStmts, scan, where);
    ast->stmts = stmts;
    return ast;
}

ast_t *ast_return(scan_t *scan, where_t where, ast_t *expr)
{
    ast_t *ast = ast_new(eAstReturn, scan, where);
    ast->operand = expr;
    return ast;
}

ast_t *ast_while(scan_t *scan, where_t where, char *label, ast_t *cond, ast_t *body, ast_t *other)
{
    ast_t *ast = ast_new(eAstWhile, scan, where);
    ast->label = label;
    ast->cond = cond;
    ast->then = body;
    ast->other = other;
    return ast;
}

ast_t *ast_branch(scan_t *scan, where_t where, ast_t *cond, ast_t *body, ast_t *other)
{
    ast_t *ast = ast_new(eAstBranch, scan, where);
    ast->cond = cond;
    ast->then = body;
    ast->other = other;
    return ast;
}

ast_t *ast_assign(scan_t *scan, where_t where, ast_t *dst, ast_t *src)
{
    ast_t *ast = ast_new(eAstAssign, scan, where);
    ast->dst = dst;
    ast->src = src;
    return ast;
}

ast_t *ast_break(scan_t *scan, where_t where, char *label)
{
    ast_t *ast = ast_new(eAstBreak, scan, where);
    ast->label = label;
    return ast;
}

ast_t *ast_continue(scan_t *scan, where_t where, char *label)
{
    ast_t *ast = ast_new(eAstContinue, scan, where);
    ast->label = label;
    return ast;
}

ast_t *ast_typename(scan_t *scan, where_t where, vector_t *path)
{
    ast_t *ast = ast_new(eAstTypename, scan, where);
    ast->path = path;
    return ast;
}

ast_t *ast_pointer(scan_t *scan, where_t where, ast_t *type, bool indexable)
{
    ast_t *ast = ast_new(eAstPointer, scan, where);
    ast->type = type;
    ast->indexable = indexable;
    return ast;
}

ast_t *ast_array(scan_t *scan, where_t where, ast_t *size, ast_t *type)
{
    ast_t *ast = ast_new(eAstArray, scan, where);
    ast->type = type;
    ast->size = size;
    return ast;
}

ast_t *ast_closure(scan_t *scan, where_t where, vector_t *params, ast_t *type)
{
    ast_t *ast = ast_new(eAstClosure, scan, where);
    ast->params = params;
    ast->result = type;
    return ast;
}

ast_t *ast_varargs(scan_t *scan, where_t where)
{
    return ast_new(eAstVarArgs, scan, where);
}

ast_t *ast_structdecl(scan_t *scan, where_t where, char *name, vector_t *fields)
{
    ast_t *ast = ast_decl(eAstDeclStruct, name, scan, where);
    ast->fields = fields;
    return ast;
}

ast_t *ast_uniondecl(scan_t *scan, where_t where, char *name, vector_t *fields)
{
    ast_t *ast = ast_decl(eAstDeclUnion, name, scan, where);
    ast->fields = fields;
    return ast;
}

ast_t *ast_typealias(scan_t *scan, where_t where, char *name, ast_t *type)
{
    ast_t *ast = ast_decl(eAstDeclAlias, name, scan, where);
    ast->alias = type;
    ast->newtype = false;
    return ast;
}

ast_t *ast_variantdecl(scan_t *scan, where_t where, char *name, ast_t *type, vector_t *fields)
{
    ast_t *ast = ast_decl(eAstDeclVariant, name, scan, where);
    ast->underlying = type;
    ast->fields = fields;
    return ast;
}

ast_t *ast_newtype(scan_t *scan, where_t where, char *name, ast_t *type)
{
    if (!ctu_has_feature(eFeatureNewTypes))
    {
        report(scan_reports(scan), eFatal, node_new(scan, where), "newtypes are unimplemented");
    }

    ast_t *ast = ast_decl(eAstDeclAlias, name, scan, where);
    ast->alias = type;
    ast->newtype = true;
    return ast;
}

ast_t *ast_field(scan_t *scan, where_t where, char *name, ast_t *type, ast_t *value)
{
    ast_t *ast = ast_decl(eAstField, name, scan, where);
    ast->field = type;
    ast->value = value;
    return ast;
}

ast_t *ast_param(scan_t *scan, where_t where, char *name, ast_t *type)
{
    ast_t *ast = ast_decl(eAstParam, name, scan, where);
    ast->param = type;
    return ast;
}

void set_attribs(ast_t *decl, bool exported, vector_t *attribs)
{
    decl->exported = exported;
    decl->attribs = attribs;
}

ast_t *ast_init(scan_t *scan, where_t where, ast_t *type, vector_t *fields)
{
    ast_t *ast = ast_new(eAstInit, scan, where);
    ast->type = type;
    ast->inits = fields;
    return ast;
}

ast_t *ast_fieldinit(scan_t *scan, where_t where, char *name, ast_t *value)
{
    ast_t *ast = ast_new(eAstFieldInit, scan, where);
    ast->name = name;
    ast->value = value;
    return ast;
}

ast_t *ast_case(scan_t *scan, where_t where, char *name, vector_t *fields, ast_t *value, bool isDefault)
{
    ast_t *ast = ast_new(eAstCase, scan, where);

    ast->name = name;
    ast->caseFields = fields;
    ast->caseValue = value;
    ast->isCaseDefault = isDefault;

    return ast;
}

ast_t *ast_default(scan_t *scan, where_t where)
{
    if (!ctu_has_feature(eFeatureDefaultExpr))
    {
        report(scan_reports(scan), eFatal, node_new(scan, where), "`default` expressions are unstable and therefore disabled by default");
    }

    return ast_new(eAstDefault, scan, where);
}
