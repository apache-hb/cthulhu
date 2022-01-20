#include "ast.h"

pl0_t *pl0_new(scan_t *scan, where_t where, pl0_type_t type) {
    pl0_t *node = ctu_malloc(sizeof(pl0_t));
    node->node = node_new(scan, where);
    node->type = type;
    return node;
}


pl0_t *pl0_digit(scan_t *scan, where_t where, mpz_t digit) {
    pl0_t *node = pl0_new(scan, where, PL0_DIGIT);
    mpz_init_set(node->digit, digit);
    return node;
}

pl0_t *pl0_ident(scan_t *scan, where_t where, const char *ident) {
    pl0_t *node = pl0_new(scan, where, PL0_IDENT);
    node->ident = ident;
    return node;
}

pl0_t *pl0_binary(scan_t *scan, where_t where, binary_t binary, pl0_t *lhs, pl0_t *rhs) {
    pl0_t *node = pl0_new(scan, where, PL0_BINARY);
    node->binary = binary;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

pl0_t *pl0_compare(scan_t *scan, where_t where, compare_t compare, pl0_t *lhs, pl0_t *rhs) {
    pl0_t *node = pl0_new(scan, where, PL0_COMPARE);
    node->compare = compare;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

pl0_t *pl0_unary(scan_t *scan, where_t where, unary_t unary, pl0_t *operand) {
    pl0_t *node = pl0_new(scan, where, PL0_UNARY);
    node->unary = unary;
    node->operand = operand;
    return node;
}

pl0_t *pl0_odd(scan_t *scan, where_t where, pl0_t *operand) {
    pl0_t *node = pl0_new(scan, where, PL0_ODD);
    node->operand = operand;
    return node;
}

pl0_t *pl0_print(scan_t *scan, where_t where, pl0_t *operand) {
    pl0_t *node = pl0_new(scan, where, PL0_PRINT);
    node->print = operand;
    return node;
}

pl0_t *pl0_assign(scan_t *scan, where_t where, const char *dst, pl0_t *src) {
    pl0_t *node = pl0_new(scan, where, PL0_ASSIGN);
    node->dst = dst;
    node->src = src;
    return node;
}

pl0_t *pl0_call(scan_t *scan, where_t where, const char *procedure) {
    pl0_t *node = pl0_new(scan, where, PL0_CALL);
    node->procedure = procedure;
    return node;
}

pl0_t *pl0_branch(scan_t *scan, where_t where, pl0_t *cond, pl0_t *then) {
    pl0_t *node = pl0_new(scan, where, PL0_BRANCH);
    node->cond = cond;
    node->then = then;
    return node;
}

pl0_t *pl0_loop(scan_t *scan, where_t where, pl0_t *cond, pl0_t *body) {
    pl0_t *node = pl0_new(scan, where, PL0_LOOP);
    node->cond = cond;
    node->then = body;
    return node;
}

pl0_t *pl0_stmts(scan_t *scan, where_t where, vector_t *stmts) {
    pl0_t *node = pl0_new(scan, where, PL0_STMTS);
    node->stmts = stmts;
    return node;
}

pl0_t *pl0_procedure(scan_t *scan, where_t where, const char *name, vector_t *locals, vector_t *body) {
    pl0_t *node = pl0_new(scan, where, PL0_PROCEDURE);
    node->name = name;
    node->locals = locals;
    node->body = body;
    return node;
}

pl0_t *pl0_value(scan_t *scan, where_t where, const char *name, pl0_t *value) {
    pl0_t *node = pl0_new(scan, where, PL0_VALUE);
    node->name = name;
    node->value = value;
    return node;
}

pl0_t *pl0_module(scan_t *scan, where_t where, const char *mod, vector_t *consts, vector_t *globals, vector_t *procs, pl0_t *entry) {
    pl0_t *node = pl0_new(scan, where, PL0_MODULE);
    node->mod = mod;
    node->consts = consts;
    node->globals = globals;
    node->procs = procs;
    node->entry = entry;
    return node;
}
