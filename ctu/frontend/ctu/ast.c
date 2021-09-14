#include "ast.h"

static ctu_t *ctu_new(scan_t *scan, where_t where, ctu_type_t type) {
    ctu_t *ctu = NEW(ctu_t);
    ctu->node = node_new(scan, where);
    ctu->type = type;
    ctu->lir = NULL;
    return ctu;
}

ctu_t *ctu_digit(scan_t *scan, where_t where, mpz_t digit) {
    ctu_t *ctu = ctu_new(scan, where, CTU_DIGIT);

    mpz_init_set(ctu->digit, digit);

    return ctu;
}

ctu_t *ctu_ident(scan_t *scan, where_t where, const char *ident) {
    ctu_t *ctu = ctu_new(scan, where, CTU_IDENT);

    ctu->ident = ident;

    return ctu;
}

ctu_t *ctu_unary(scan_t *scan, where_t where, unary_t unary, ctu_t *operand) {
    ctu_t *ctu = ctu_new(scan, where, CTU_UNARY);

    ctu->unary = unary;
    ctu->operand = operand;

    return ctu;
}

ctu_t *ctu_binary(scan_t *scan, where_t where, binary_t binary, ctu_t *lhs, ctu_t *rhs) {
    ctu_t *ctu = ctu_new(scan, where, CTU_BINARY);

    ctu->binary = binary;
    ctu->lhs = lhs;
    ctu->rhs = rhs;

    return ctu;
}

ctu_t *ctu_value(scan_t *scan, where_t where, const char *name, ctu_t *value) {
    ctu_t *ctu = ctu_new(scan, where, CTU_VALUE);

    ctu->name = name;
    ctu->value = value;

    return ctu;
}

ctu_t *ctu_module(scan_t *scan, where_t where, vector_t *decls) {
    ctu_t *ctu = ctu_new(scan, where, CTU_MODULE);

    ctu->decls = decls;

    return ctu;
}
