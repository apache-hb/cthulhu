#include "ast.h"

pl0_node_t *pl0_new(scan_t *scan, where_t where, pl0_ast_t type) {
    pl0_node_t *node = ctu_malloc(sizeof(pl0_node_t));

    node->type = type;
    node->where = where;
    node->scan = scan;

    return node;
}

pl0_node_t *pl0_program(scan_t *scan, where_t where, vector_t *consts, vector_t *vars) {
    pl0_node_t *node = pl0_new(scan, where, PL0_PROGRAM);

    node->consts = consts;
    node->vars = vars;

    return node;
}

pl0_node_t *pl0_const(scan_t *scan, where_t where, pl0_node_t *key, pl0_node_t *value) {
    pl0_node_t *node = pl0_new(scan, where, PL0_GLOBAL);

    node->key = key;
    node->value = value;

    return node;
}

pl0_node_t *pl0_ident(scan_t *scan, where_t where, char *ident) {
    pl0_node_t *node = pl0_new(scan, where, PL0_IDENT);

    node->ident = ident;

    return node;
}

pl0_node_t *pl0_number(scan_t *scan, where_t where, mpz_t number) {
    pl0_node_t *node = pl0_new(scan, where, PL0_NUMBER);

    mpz_init_set(node->number, number);

    return node;
}
