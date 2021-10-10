#include "ast.h"

static ctu_t *ctu_new(scan_t *scan, where_t where, ctu_type_t type) {
    ctu_t *ctu = ctu_malloc(sizeof(ctu_t));
    ctu->node = node_new(scan, where);
    ctu->type = type;
    ctu->lir = NULL;
    return ctu;
}

static ctu_t *ctu_decl(scan_t *scan, where_t where, ctu_type_t type, const char *name) {
    ctu_t *ctu = ctu_new(scan, where, type);
    ctu->name = name;
    ctu->attribs = vector_new(0);
    ctu->exported = false;
    return ctu;
}

ctu_t *ctu_digit(scan_t *scan, where_t where, mpz_t digit) {
    ctu_t *ctu = ctu_new(scan, where, CTU_DIGIT);

    mpz_init_set(ctu->digit, digit);

    return ctu;
}

ctu_t *ctu_bool(scan_t *scan, where_t where, bool value) {
    ctu_t *ctu = ctu_new(scan, where, CTU_BOOL);

    ctu->boolean = value;

    return ctu;
}

ctu_t *ctu_ident(scan_t *scan, where_t where, const char *ident) {
    ctu_t *ctu = ctu_new(scan, where, CTU_IDENT);

    ctu->ident = ident;

    return ctu;
}

ctu_t *ctu_string(scan_t *scan, where_t where, const char *str) {
    ctu_t *ctu = ctu_new(scan, where, CTU_STRING);

    ctu->str = str;

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

ctu_t *ctu_call(scan_t *scan, where_t where, ctu_t *func, vector_t *args) {
    ctu_t *ctu = ctu_new(scan, where, CTU_CALL);

    ctu->func = func;
    ctu->args = args;

    return ctu;
}

ctu_t *ctu_stmts(scan_t *scan, where_t where, vector_t *stmts) {
    ctu_t *ctu = ctu_new(scan, where, CTU_STMTS);

    ctu->stmts = stmts;

    return ctu;
}

ctu_t *ctu_return(scan_t *scan, where_t where, ctu_t *operand) {
    ctu_t *ctu = ctu_new(scan, where, CTU_RETURN);
    ctu->operand = operand;
    return ctu;
}

ctu_t *ctu_while(scan_t *scan, where_t where, ctu_t *cond, ctu_t *body) {
    ctu_t *ctu = ctu_new(scan, where, CTU_WHILE);

    ctu->cond = cond;
    ctu->then = body;

    return ctu;
}

ctu_t *ctu_assign(scan_t *scan, where_t where, ctu_t *dst, ctu_t *src) {
    ctu_t *ctu = ctu_new(scan, where, CTU_ASSIGN);
    
    ctu->dst = dst;
    ctu->src = src;

    return ctu;
}

ctu_t *ctu_branch(scan_t *scan, where_t where, ctu_t *cond, ctu_t *then) {
    ctu_t *ctu = ctu_new(scan, where, CTU_BRANCH);

    ctu->cond = cond;
    ctu->then = then;

    return ctu;
}

ctu_t *ctu_pointer(scan_t *scan, where_t where, 
                   ctu_t *ptr)
{
    ctu_t *ctu = ctu_new(scan, where, CTU_POINTER);

    ctu->ptr = ptr;

    return ctu;
}

ctu_t *ctu_typename(scan_t *scan, where_t where, 
                    const char *name) 
{
    ctu_t *ctu = ctu_new(scan, where, CTU_TYPENAME);

    ctu->ident = name;

    return ctu;
}

ctu_t *ctu_value(scan_t *scan, where_t where, 
                 const char *name, ctu_t *type,
                 ctu_t *value) {
    ctu_t *ctu = ctu_decl(scan, where, CTU_VALUE, name);

    ctu->kind = type;
    ctu->value = value;

    return ctu;
}

ctu_t *ctu_param(scan_t *scan, where_t where,
                 const char *name, ctu_t *type)
{
    ctu_t *ctu = ctu_decl(scan, where, CTU_PARAM, name);

    ctu->kind = type;

    return ctu;
}

ctu_t *ctu_define(scan_t *scan, where_t where, 
                  const char *name, vector_t *params, 
                  ctu_t *result, ctu_t *body)
{
    ctu_t *ctu = ctu_decl(scan, where, CTU_DEFINE, name);

    ctu->params = params;
    ctu->result = result;
    ctu->body = body;

    return ctu;
}

ctu_t *ctu_attrib(scan_t *scan, where_t where, const char *name, vector_t *params) {
    ctu_t *ctu = ctu_decl(scan, where, CTU_ATTRIB, name);
    ctu->params = params;
    return ctu;
}

ctu_t *ctu_module(scan_t *scan, where_t where, vector_t *decls) {
    ctu_t *ctu = ctu_new(scan, where, CTU_MODULE);

    ctu->decls = decls;

    return ctu;
}

ctu_t *set_details(ctu_t *decl, vector_t *attribs, bool exported) {
    decl->attribs = attribs;
    decl->exported = exported;
    return decl;
}
