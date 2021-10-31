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

ctu_t *ctu_path(scan_t *scan, where_t where, vector_t *path) {
    ctu_t *ctu = ctu_new(scan, where, CTU_PATH);

    ctu->path = path;

    return ctu;
}

ctu_t *ctu_string(scan_t *scan, where_t where, const char *str) {
    ctu_t *ctu = ctu_new(scan, where, CTU_STRING);

    ctu->str = str;

    return ctu;
}

ctu_t *ctu_null(scan_t *scan, where_t where) {
    return ctu_new(scan, where, CTU_NULL);
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

ctu_t *ctu_cast(scan_t *scan, where_t where, ctu_t *expr, ctu_t *type) {
    ctu_t *ctu = ctu_new(scan, where, CTU_CAST);

    ctu->src = expr;
    ctu->dst = type;

    return ctu;
}

ctu_t *ctu_lambda(scan_t *scan, where_t where, vector_t *params, ctu_t *result, ctu_t *body) {
    ctu_t *ctu = ctu_new(scan, where, CTU_LAMBDA);

    ctu->params = params;
    ctu->result = result;
    ctu->body = body;

    return ctu;
}

ctu_t *ctu_index(scan_t *scan, where_t where, ctu_t *array, ctu_t *index) {
    ctu_t *ctu = ctu_new(scan, where, CTU_INDEX);

    ctu->array = array;
    ctu->index = index;

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

ctu_t *ctu_branch(scan_t *scan, where_t where, ctu_t *cond, ctu_t *then, ctu_t *other) {
    ctu_t *ctu = ctu_new(scan, where, CTU_BRANCH);

    ctu->cond = cond;
    ctu->then = then;
    ctu->other = other;

    return ctu;
}

ctu_t *ctu_break(scan_t *scan, where_t where) {
    return ctu_new(scan, where, CTU_BREAK);
}

ctu_t *ctu_pointer(scan_t *scan, where_t where, ctu_t *ptr, bool subscript) {
    ctu_t *ctu = ctu_new(scan, where, CTU_POINTER);

    ctu->ptr = ptr;
    ctu->subscript = subscript;

    return ctu;
}

ctu_t *ctu_typename(scan_t *scan, where_t where, const char *name) {
    return ctu_typepath(scan, where, vector_init((char*)name));
}

ctu_t *ctu_typepath(scan_t *scan, where_t where, vector_t *path) {
    ctu_t *ctu = ctu_new(scan, where, CTU_TYPEPATH);
    ctu->path = path;
    return ctu;
}

ctu_t *ctu_closure(scan_t *scan, where_t where,
                    vector_t *args, ctu_t *result)
{
    ctu_t *ctu = ctu_new(scan, where, CTU_CLOSURE);

    ctu->params = args;
    ctu->result = result;

    return ctu;
}

ctu_t *ctu_mutable(scan_t *scan, where_t where, ctu_t *type) {
    ctu_t *ctu = ctu_new(scan, where, CTU_MUTABLE);

    ctu->kind = type;

    return ctu;
}

ctu_t *ctu_varargs(scan_t *scan, where_t where) {
    return ctu_new(scan, where, CTU_VARARGS);
}

ctu_t *ctu_array(scan_t *scan, where_t where, ctu_t *type, ctu_t *size) {
    ctu_t *ctu = ctu_new(scan, where, CTU_ARRAY);

    ctu->arr = type;
    ctu->size = size;

    return ctu;
}

ctu_t *ctu_value(scan_t *scan, where_t where, 
                 bool mut, const char *name, 
                 ctu_t *type, ctu_t *value) {
    ctu_t *ctu = ctu_decl(scan, where, CTU_VALUE, name);

    ctu->mut = mut;
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

ctu_t *ctu_newtype(scan_t *scan, where_t where, 
                   const char *name, ctu_t *type) 
{
    ctu_t *ctu = ctu_decl(scan, where, CTU_NEWTYPE, name);
    ctu->result = type;
    return ctu;
}

ctu_t *ctu_attrib(scan_t *scan, where_t where, const char *name, vector_t *params) {
    ctu_t *ctu = ctu_decl(scan, where, CTU_ATTRIB, name);
    ctu->params = params;
    return ctu;
}

ctu_t *ctu_module(scan_t *scan, where_t where, vector_t *imports, vector_t *decls) {
    ctu_t *ctu = ctu_new(scan, where, CTU_MODULE);

    ctu->imports = imports;
    ctu->decls = decls;

    return ctu;
}

ctu_t *ctu_import(scan_t *scan, where_t where, vector_t *path, const char *alias) {
    ctu_t *ctu = ctu_new(scan, where, CTU_IMPORT);

    ctu->path = path;
    ctu->alias = alias;

    return ctu;
}

ctu_t *set_details(ctu_t *decl, vector_t *attribs, bool exported) {
    decl->attribs = attribs;
    decl->exported = exported;
    return decl;
}
