#include "ast.h"

pl0_node_t *pl0_new(scan_t *scan, where_t where, pl0_ast_t type) {
    pl0_node_t *node = ctu_malloc(sizeof(pl0_node_t));

    node->type = type;
    node->where = where;
    node->scan = scan;

    return node;
}

pl0_node_t *pl0_program(scan_t *scan, where_t where, vector_t *consts) {
    pl0_node_t *node = pl0_new(scan, where, PL0_PROGRAM);

    node->consts = consts;

    return node;
}

pl0_node_t *pl0_const(scan_t *scan, where_t where, char *key, mpz_t value) {
    pl0_node_t *node = pl0_new(scan, where, PL0_CONST);

    node->key = key;
    mpz_init_set(node->value, value);

    return node;
}
