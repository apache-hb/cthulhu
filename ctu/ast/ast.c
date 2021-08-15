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

node_t *ast_ident(scan_t *scan, where_t where, char *ident) {
    node_t *node = ast_new(scan, where, AST_IDENT);

    node->ident = ident;

    return node;
}
