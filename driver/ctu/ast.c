#include "ast.h"

#include "cthulhu/util/util.h"

static const where_t kNowhere = {0, 0, 0, 0};

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
    return ast;
}

ast_t *ast_program(scan_t *scan, where_t where, ast_t *modspec, vector_t *imports, vector_t *decls)
{
    ast_t *ast = ast_new(AST_PROGRAM, scan, where);
    ast->modspec = modspec;
    ast->imports = imports;
    ast->decls = decls;
    return ast;
}

ast_t *ast_import(scan_t *scan, where_t where, vector_t *path)
{
    ast_t *ast = ast_new(AST_IMPORT, scan, where);
    ast->path = path;
    return ast;
}

ast_t *ast_module(scan_t *scan, where_t where, vector_t *path)
{
    ast_t *ast = ast_new(AST_MODULE, scan, where);
    ast->path = path;
    return ast;
}

ast_t *ast_funcdecl(scan_t *scan, where_t where, char *name, ast_t *ret, ast_t *body)
{
    ast_t *ast = ast_new(AST_FUNCDECL, scan, where);
    ast->name = name;
    ast->ret = ret;
    ast->body = body;
    return ast;
}

ast_t *ast_digit(scan_t *scan, where_t where, mpz_t value)
{
    ast_t *ast = ast_new(AST_DIGIT, scan, where);
    mpz_init_set(ast->digit, value);
    return ast;
}

ast_t *ast_bool(scan_t *scan, where_t where, bool value)
{
    ast_t *ast = ast_new(AST_BOOL, scan, where);
    ast->boolean = value;
    return ast;
}

ast_t *ast_name(scan_t *scan, where_t where, vector_t *path)
{
    ast_t *ast = ast_new(AST_TYPENAME, scan, where);
    ast->path = path;
    return ast;
}

ast_t *ast_binary(scan_t *scan, where_t where, binary_t binary, ast_t *lhs, ast_t *rhs)
{
    ast_t *ast = ast_new(AST_BINARY, scan, where);
    ast->binary = binary;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

ast_t *ast_stmts(scan_t *scan, where_t where, vector_t *stmts)
{
    ast_t *ast = ast_new(AST_STMTS, scan, where);
    ast->stmts = stmts;
    return ast;
}

ast_t *ast_return(scan_t *scan, where_t where, ast_t *expr)
{
    ast_t *ast = ast_new(AST_RETURN, scan, where);
    ast->operand = expr;
    return ast;
}

ast_t *ast_typename(scan_t *scan, where_t where, vector_t *path)
{
    ast_t *ast = ast_new(AST_TYPENAME, scan, where);
    ast->path = path;
    return ast;
}

ast_t *ast_pointer(scan_t *scan, where_t where, ast_t *type, bool indexable)
{
    ast_t *ast = ast_new(AST_POINTER, scan, where);
    ast->type = type;
    ast->indexable = indexable;
    return ast;
}

ast_t *ast_array(scan_t *scan, where_t where, ast_t *size, ast_t *type)
{
    ast_t *ast = ast_new(AST_ARRAY, scan, where);
    ast->type = type;
    ast->size = size;
    return ast;
}

ast_t *ast_closure(scan_t *scan, where_t where, ast_t *args, ast_t *type)
{
    args->of = AST_CLOSURE;
    args->node = node_new(scan, where);
    args->result = type;
    return args;
}

ast_t *ast_typelist(vector_t *types, bool variadic)
{
    ast_t *ast = ast_new(AST_TYPELIST, NULL, kNowhere);
    ast->params = types;
    ast->variadic = variadic;
    return ast;
}

ast_t *ast_structdecl(scan_t *scan, where_t where, char *name, vector_t *fields)
{
    ast_t *ast = ast_decl(AST_STRUCTDECL, name, scan, where);
    ast->fields = fields;
    return ast;
}

ast_t *ast_uniondecl(scan_t *scan, where_t where, char *name, vector_t *fields)
{
    ast_t *ast = ast_decl(AST_UNIONDECL, name, scan, where);
    ast->fields = fields;
    return ast;
}

ast_t *ast_typealias(scan_t *scan, where_t where, char *name, ast_t *type)
{
    ast_t *ast = ast_decl(AST_ALIASDECL, name, scan, where);
    ast->alias = type;
    return ast;
}

ast_t *ast_variantdecl(scan_t *scan, where_t where, char *name, vector_t *fields)
{
    ast_t *ast = ast_decl(AST_VARIANTDECL, name, scan, where);
    ast->fields = fields;
    return ast;
}

ast_t *ast_field(scan_t *scan, where_t where, char *name, ast_t *type)
{
    ast_t *ast = ast_decl(AST_FIELD, name, scan, where);
    ast->field = type;
    return ast;
}
