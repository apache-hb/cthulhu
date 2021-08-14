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

node_t *ast_value(scan_t *scan, where_t where, char *name, node_t *value) {
    node_t *node = ast_new(scan, where, AST_DECL_VALUE);

    node->name = name;
    node->value = value;

    return node;
}
