#include "ast.h"

static pl0_t *pl0_new(scan_t *scan, where_t where, pl0_type_t type) {    
    pl0_t *pl0 = NEW(pl0_t);
    pl0->node = node_new(scan, where);
    pl0->type = type;
    return pl0;
}


pl0_t *pl0_digit(scan_t *scan, where_t where, mpz_t value) {
    pl0_t *pl0 = pl0_new(scan, where, PL0_DIGIT);

    mpz_init_set(pl0->digit, value);

    return pl0;
}

pl0_t *pl0_ident(scan_t *scan, where_t where, const char *name) {
    pl0_t *pl0 = pl0_new(scan, where, PL0_IDENT);

    pl0->ident = name;

    return pl0;
}

pl0_t *pl0_binary(scan_t *scan, where_t where, 
                  binary_t binary, pl0_t *left, pl0_t *right)
{
    pl0_t *pl0 = pl0_new(scan, where, PL0_BINARY);

    pl0->binary = binary;
    pl0->lhs = left;
    pl0->rhs = right;

    return pl0;
}

pl0_t *pl0_unary(scan_t *scan, where_t where,
                 unary_t unary, pl0_t *operand)
{
    pl0_t *pl0 = pl0_new(scan, where, PL0_UNARY);

    pl0->unary = unary;
    pl0->operand = operand;

    return pl0;
}

pl0_t *pl0_odd(scan_t *scan, where_t where, pl0_t *operand) {
    pl0_t *pl0 = pl0_new(scan, where, PL0_ODD);

    pl0->operand = operand;

    return pl0;
}

pl0_t *pl0_assign(scan_t *scan, where_t where,
                  const char *dst, pl0_t *src)
{
    pl0_t *pl0 = pl0_new(scan, where, PL0_ASSIGN);

    pl0->dst = dst;
    pl0->src = src;

    return pl0;
}

pl0_t *pl0_call(scan_t *scan, where_t where, const char *proc) {
    pl0_t *pl0 = pl0_new(scan, where, PL0_CALL);

    pl0->ident = proc;

    return pl0;
}

pl0_t *pl0_branch(scan_t *scan, where_t where,
                  pl0_t *cond, pl0_t *then)
{
    pl0_t *pl0 = pl0_new(scan, where, PL0_BRANCH);

    pl0->cond = cond;
    pl0->then = then;

    return pl0;
}

pl0_t *pl0_loop(scan_t *scan, where_t where,
                 pl0_t *cond, pl0_t *body)
{
    pl0_t *pl0 = pl0_new(scan, where, PL0_LOOP);

    pl0->cond = cond;
    pl0->then = body;

    return pl0;
}

pl0_t *pl0_print(scan_t *scan, where_t where,
                 pl0_t *value)
{
    pl0_t *pl0 = pl0_new(scan, where, PL0_PRINT);

    pl0->operand = value;

    return pl0;
}

pl0_t *pl0_stmts(scan_t *scan, where_t where,
                 vector_t *stmts)
{
    pl0_t *pl0 = pl0_new(scan, where, PL0_STMTS);

    pl0->stmts = stmts;

    return pl0;
}

pl0_t *pl0_value(scan_t *scan, where_t where, 
                 const char *name, pl0_t *value) 
{
    pl0_t *pl0 = pl0_new(scan, where, PL0_VALUE);

    pl0->name = name;
    pl0->value = value;

    return pl0;
}

pl0_t *pl0_procedure(scan_t *scan, where_t where,
                     const char *name, vector_t *locals,
                     vector_t *body)
{
    pl0_t *pl0 = pl0_new(scan, where, PL0_PROCEDURE);

    pl0->name = name;
    pl0->locals = locals;
    pl0->body = body;

    return pl0;
}

pl0_t *pl0_module(scan_t *scan, where_t where, 
                  vector_t *consts, vector_t *globals, 
                  vector_t *procs, struct pl0_t *toplevel)
{
    pl0_t *pl0 = pl0_new(scan, where, PL0_MODULE);

    pl0->consts = consts;
    pl0->globals = globals;
    pl0->procs = procs;
    pl0->toplevel = toplevel;

    return pl0;
}
