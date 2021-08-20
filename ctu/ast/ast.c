#include "ast.h"

#include "ctu/util/util.h"

static node_t *ast_new(scan_t *scan, where_t where, ast_t kind) {
    node_t *node = ctu_malloc(sizeof(node_t));

    node->kind = kind;
    node->scan = scan;
    node->where = where;

    return node;
}

static node_t *ast_decl(scan_t *scan, where_t where, ast_t kind, node_t *name) {
    node_t *node = ast_new(scan, where, kind);

    node->name = name;

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

node_t *ast_call(scan_t *scan, where_t where, node_t *call, vector_t *args) {
    node_t *node = ast_new(scan, where, AST_CALL);

    node->call = call;
    node->args = args;

    return node;
}

node_t *ast_assign(scan_t *scan, where_t where, node_t *dst, node_t *src) {
    node_t *node = ast_new(scan, where, AST_ASSIGN);

    node->dst = dst;
    node->src = src;

    return node;
}

node_t *ast_while(scan_t *scan, where_t where, node_t *cond, node_t *then) {
    node_t *node = ast_new(scan, where, AST_WHILE);

    node->cond = cond;
    node->then = then;
    node->other = NULL;

    return node;
}

node_t *ast_branch(scan_t *scan, where_t where, node_t *cond, node_t *then, node_t *other) {
    node_t *node = ast_new(scan, where, AST_BRANCH);

    node->cond = cond;
    node->then = then;
    node->other = other;

    return node;
}

node_t *ast_stmts(scan_t *scan, where_t where, vector_t *stmts) {
    node_t *node = ast_new(scan, where, AST_STMTS);

    node->stmts = stmts;

    return node;
}

node_t *ast_value(scan_t *scan, where_t where, node_t *name, node_t *type, node_t *value) {
    node_t *node = ast_decl(scan, where, AST_VALUE, name);

    node->type = type;
    node->value = value;

    return node;
}

node_t *ast_define(scan_t *scan, where_t where, node_t *name, 
    vector_t *locals, vector_t *params, 
    node_t *result, node_t *body) {
    node_t *node = ast_decl(scan, where, AST_DEFINE, name);

    node->locals = locals;
    node->params = params;
    node->result = result;
    node->body = body;

    return node;
}

node_t *ast_module(scan_t *scan, where_t where, vector_t *values, vector_t *defines) {
    node_t *node = ast_new(scan, where, AST_MODULE);

    node->values = values;
    node->defines = defines;

    return node;
}
