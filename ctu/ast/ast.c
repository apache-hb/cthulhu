#include "ast.h"

#include "ctu/util/util.h"

static node_t *ast_new(scan_t *scan, where_t where, ast_t kind) {
    node_t *node = ctu_malloc(sizeof(node_t));

    node->kind = kind;
    node->scan = scan;
    node->where = where;

    return node;
}

node_t *ast_digit(scan_t *scan, where_t where, mpz_t digit) {
    node_t *node = ast_new(scan, where, AST_DIGIT);

    mpz_init_set(node->digit, digit);

    return node;
}

node_t *ast_int(scan_t *scan, where_t where, int digit) {
    node_t *node = ast_new(scan, where, AST_DIGIT);

    mpz_init_set_si(node->digit, digit);

    return node;
}

node_t *ast_ident(scan_t *scan, where_t where, char *ident) {
    node_t *node = ast_new(scan, where, AST_IDENT);

    node->ident = ident;

    return node;
}

node_t *ast_unary(scan_t *scan, where_t where, unary_t unary, node_t *operand) {
    node_t *node = ast_new(scan, where, AST_UNARY);

    node->unary = unary;
    node->operand = operand;

    return node;
}

node_t *ast_binary(scan_t *scan, where_t where, binary_t binary, node_t *lhs, node_t *rhs) {
    node_t *node = ast_new(scan, where, AST_BINARY);

    node->binary = binary;
    node->lhs = lhs;
    node->rhs = rhs;

    return node;
}
