#include "ast.h"

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

static att_t *att_new(attof_t of, scan_t *scan, where_t where) {
    att_t *att = ctu_malloc(sizeof(att_t));
    att->of = of;
    att->node = node_new(scan, where);
    return att;
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

ast_t *ast_vardecl(scan_t *scan, where_t where, char *name, att_t *type, ast_t *init) {
    ast_t *ast = ast_decl(AST_VAR, name, scan, where);
    ast->type = type;
    ast->init = init;
    return ast;
}

ast_t *ast_name(scan_t *scan, where_t where, vector_t *path) {
    ast_t *ast = ast_new(AST_NAME, scan, where);
    ast->path = path;
    return ast;
}

ast_t *ast_digit(scan_t *scan, where_t where, mpz_t value) {
    ast_t *ast = ast_new(AST_DIGIT, scan, where);
    mpz_init_set(ast->digit, value);
    return ast;
}

ast_t *ast_cast(scan_t *scan, where_t where, ast_t *operand, att_t *cast) {
    ast_t *ast = ast_new(AST_CAST, scan, where);
    ast->operand = operand;
    ast->cast = cast;
    return ast;
}

att_t *att_name(scan_t *scan, where_t where, vector_t *path) {
    att_t *att = att_new(ATT_NAME, scan, where);
    att->path = path;
    return att;
}

att_t *att_ptr(scan_t *scan, where_t where, att_t *ptr) {
    att_t *att = att_new(ATT_PTR, scan, where);
    att->ptr = ptr;
    return att;
}
