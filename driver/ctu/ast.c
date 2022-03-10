#include "ast.h"

static where_t NOWHERE = { 0, 0, 0, 0 };

static ast_t *ast_new(astof_t of, scan_t *scan, where_t where) {
    ast_t *ast = ctu_malloc(sizeof(ast_t));
    ast->of = of;
    ast->node = node_new(scan, where);
    return ast;
}

static ast_t *ast_decl(astof_t of, char *name, scan_t *scan, where_t where) {
    ast_t *ast = ast_new(of, scan, where);
    ast->name = name;
    return ast;
}

ast_t *ast_program(scan_t *scan, where_t where, ast_t *modspec, vector_t *decls) {
    ast_t *ast = ast_new(AST_PROGRAM, scan, where);
    ast->modspec = modspec;
    ast->decls = decls;
    return ast;
}

ast_t *ast_module(scan_t *scan, where_t where, vector_t *path) {
    ast_t *ast = ast_new(AST_MODULE, scan, where);
    ast->path = path;
    return ast;
}

ast_t *ast_typename(scan_t *scan, where_t where, vector_t *path) {
    ast_t *ast = ast_new(AST_TYPENAME, scan, where);
    ast->path = path;
    return ast;
}

ast_t *ast_pointer(scan_t *scan, where_t where, ast_t *type, bool indexable) {
    ast_t *ast = ast_new(AST_POINTER, scan, where);
    ast->type = type;
    ast->indexable = indexable;
    return ast;
}

ast_t *ast_array(scan_t *scan, where_t where, ast_t *type, ast_t *size) {
    ast_t *ast = ast_new(AST_ARRAY, scan, where);
    ast->type = type;
    ast->size = size;
    return ast;
}

ast_t *ast_closure(scan_t *scan, where_t where, ast_t *args, ast_t *type) {
    args->of = AST_CLOSURE;
    args->node = node_new(scan, where);
    args->result = type;
    return args;
}

ast_t *ast_typelist(vector_t *types, bool variadic) {
    ast_t *ast = ast_new(AST_TYPELIST, NULL, NOWHERE);
    ast->params = types;
    ast->variadic = variadic;
    return ast;
}

ast_t *ast_structdecl(scan_t *scan, where_t where, char *name, vector_t *fields) {
    ast_t *ast = ast_decl(AST_STRUCTDECL, name, scan, where);
    ast->fields = fields;
    return ast;
}

ast_t *ast_uniondecl(scan_t *scan, where_t where, char *name, vector_t *fields) {
    ast_t *ast = ast_decl(AST_UNIONDECL, name, scan, where);
    ast->fields = fields;
    return ast;
}

ast_t *ast_typealias(scan_t *scan, where_t where, char *name, ast_t *type) {
    ast_t *ast = ast_decl(AST_ALIASDECL, name, scan, where);
    ast->alias = type;
    return ast;
}

ast_t *ast_field(scan_t *scan, where_t where, char *name, ast_t *type) {
    ast_t *ast = ast_decl(AST_FIELD, name, scan, where);
    ast->field = type;
    return ast;
}
