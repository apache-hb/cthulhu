#include "ctu/ast.h"

#include "base/memory.h"

static ctu_t *ctu_new(scan_t *scan, where_t where, ctu_kind_t kind)
{
    ctu_t *self = ctu_malloc(sizeof(ctu_t));
    self->kind = kind;
    self->node = node_new(scan, where);
    return self;
}

static ctu_t *ctu_decl(scan_t *scan, where_t where, ctu_kind_t kind, char *name, bool exported)
{
    ctu_t *self = ctu_new(scan, where, kind);
    self->name = name;
    self->exported = exported;
    return self;
}

ctu_t *ctu_module(scan_t *scan, where_t where, vector_t *modspec, vector_t *imports, vector_t *decls)
{
    ctu_t *ast = ctu_new(scan, where, eCtuModule);
    ast->modspec = modspec;
    ast->imports = imports;
    ast->decls = decls;
    return ast;
}

ctu_t *ctu_import(scan_t *scan, where_t where, vector_t *path, char *name)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuImport, name, false);
    ast->importPath = path;
    return ast;
}

/* stmts */

ctu_t *ctu_stmt_list(scan_t *scan, where_t where, vector_t *stmts)
{
    ctu_t *ast = ctu_new(scan, where, eCtuStmtList);
    ast->stmts = stmts;
    return ast;
}

ctu_t *ctu_stmt_local(scan_t *scan, where_t where, bool mutable, char *name, ctu_t *type, ctu_t *value)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuStmtLocal, name, false);
    ast->mut = mutable;
    ast->type = type;
    ast->value = value;
    return ast;
}

ctu_t *ctu_stmt_return(scan_t *scan, where_t where, ctu_t *value)
{
    ctu_t *ast = ctu_new(scan, where, eCtuStmtReturn);
    ast->result = value;
    return ast;
}

ctu_t *ctu_stmt_while(scan_t *scan, where_t where, ctu_t *cond, ctu_t *then, ctu_t *other)
{
    ctu_t *ast = ctu_new(scan, where, eCtuStmtWhile);
    ast->cond = cond;
    ast->then = then;
    ast->other = other;
    return ast;
}

ctu_t *ctu_stmt_assign(scan_t *scan, where_t where, ctu_t *dst, ctu_t *src)
{
    ctu_t *ast = ctu_new(scan, where, eCtuStmtAssign);
    ast->dst = dst;
    ast->src = src;
    return ast;
}

/* exprs */

ctu_t *ctu_expr_int(scan_t *scan, where_t where, mpz_t value)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprInt);
    mpz_init_set(ast->intValue, value);
    return ast;
}

ctu_t *ctu_expr_bool(scan_t *scan, where_t where, bool value)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprBool);
    ast->boolValue = value;
    return ast;
}

ctu_t *ctu_expr_string(scan_t *scan, where_t where, char *text, size_t length)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprString);
    ast->text = text;
    ast->length = length;
    return ast;
}

ctu_t *ctu_expr_name(scan_t *scan, where_t where, vector_t *path)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprName);
    ast->path = path;
    return ast;
}

ctu_t *ctu_expr_ref(scan_t *scan, where_t where, ctu_t *expr)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprRef);
    ast->expr = expr;
    return ast;
}

ctu_t *ctu_expr_deref(scan_t *scan, where_t where, ctu_t *expr)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprDeref);
    ast->expr = expr;
    return ast;
}

ctu_t *ctu_expr_unary(scan_t *scan, where_t where, unary_t unary, ctu_t *expr)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprUnary);
    ast->unary = unary;
    ast->expr = expr;
    return ast;
}

ctu_t *ctu_expr_binary(scan_t *scan, where_t where, binary_t binary, ctu_t *lhs, ctu_t *rhs)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprBinary);
    ast->binary = binary;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

ctu_t *ctu_expr_compare(scan_t *scan, where_t where, compare_t compare, ctu_t *lhs, ctu_t *rhs)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprCompare);
    ast->compare = compare;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}


/* types */

ctu_t *ctu_type_name(scan_t *scan, where_t where, vector_t *path)
{
    ctu_t *ast = ctu_new(scan, where, eCtuTypeName);
    ast->typeName = path;
    return ast;
}

ctu_t *ctu_type_pointer(scan_t *scan, where_t where, ctu_t *pointer)
{
    ctu_t *ast = ctu_new(scan, where, eCtuTypePointer);
    ast->pointer = pointer;
    return ast;
}

/* decls */

ctu_t *ctu_decl_global(scan_t *scan, where_t where, bool exported, bool mut, char *name, ctu_t *type, ctu_t *value)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuDeclGlobal, name, exported);
    ast->mut = mut;
    ast->type = type;
    ast->value = value;
    return ast;
}

ctu_t *ctu_decl_function(scan_t *scan, where_t where, bool exported, char *name, vector_t *params, ctu_t *returnType, ctu_t *body)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuDeclFunction, name, exported);
    ast->params = params;
    ast->returnType = returnType;
    ast->body = body;
    return ast;
}

/* type decls */

ctu_t *ctu_decl_typealias(scan_t *scan, where_t where, bool exported, char *name, bool newtype, ctu_t *type)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuDeclTypeAlias, name, exported);
    ast->newtype = newtype;
    ast->typeAlias = type;
    return ast;
}

ctu_t *ctu_decl_struct(scan_t *scan, where_t where, bool exported, char *name, vector_t *fields)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuDeclStruct, name, exported);
    ast->fields = fields;
    return ast;
}

ctu_t *ctu_field(scan_t *scan, where_t where, char *name, ctu_t *type)
{
    ctu_t *ast = ctu_new(scan, where, eCtuField);
    ast->name = name;
    ast->fieldType = type;
    return ast;
}

ctu_t *ctu_param(scan_t *scan, where_t where, char *name, ctu_t *type)
{
    ctu_t *ast = ctu_new(scan, where, eCtuParam);
    ast->name = name;
    ast->paramType = type;
    return ast;
}
