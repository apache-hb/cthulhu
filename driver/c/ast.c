#include "ast.h"

static ast_t *new_ast(astof_t type, scan_t *scan, where_t where) {
    ast_t *ast = ctu_malloc(sizeof(ast_t));
    ast->type = type;
    ast->node = node_new(scan, where);
    return ast;
}

ast_t *ast_digit(scan_t *scan, where_t where, mpz_t digit) {
    ast_t *ast = new_ast(AST_DIGIT, scan, where);
    mpz_init_set(ast->digit, digit);
    return ast;
}
