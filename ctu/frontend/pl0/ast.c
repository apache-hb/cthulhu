#include "ast.h"

static pl0_node_t *pl0_new(scan_t *scan, where_t where, pl0_ast_t type) {
    pl0_node_t *node = ctu_malloc(sizeof(pl0_node_t));

    node->type = type;
    node->where = where;
    node->scan = scan;

    return node;
}

static pl0_node_t *pl0_decl(scan_t *scan, where_t where, pl0_ast_t type, node_t *name) {
    pl0_node_t *node = pl0_new(scan, where, type);

    node->name = name;

    return node;
}

pl0_node_t *pl0_program(scan_t *scan, where_t where, vector_t *consts, vector_t *vars, vector_t *procedures) {
    pl0_node_t *node = pl0_new(scan, where, PL0_PROGRAM);

    node->consts = consts;
    node->vars = vars;
    node->procedures = procedures;

    return node;
}

pl0_node_t *pl0_value(scan_t *scan, where_t where, node_t *name) {
    return pl0_decl(scan, where, PL0_GLOBAL, name);
}

pl0_node_t *pl0_const(scan_t *scan, where_t where, node_t *name, node_t *value) {
    pl0_node_t *node = pl0_decl(scan, where, PL0_CONST, name);

    node->value = value;

    return node;
}

pl0_node_t *pl0_procedure(scan_t *scan, where_t where, node_t *name, vector_t *locals) {
    pl0_node_t *node = pl0_decl(scan, where, PL0_PROCEDURE, name);

    node->locals = locals;

    return node;
}

pl0_node_t *pl0_statements(scan_t *scan, where_t where, vector_t *statements) {
    pl0_node_t *node = pl0_new(scan, where, PL0_STATEMENTS);

    node->statements = statements;

    return node;
}

pl0_node_t *pl0_call(scan_t *scan, where_t where, node_t *call) {
    pl0_node_t *node = pl0_new(scan, where, PL0_CALL);

    node->call = call;

    return node;
}

pl0_node_t *pl0_assign(scan_t *scan, where_t where, node_t *dst, node_t *src) {
    pl0_node_t *node = pl0_new(scan, where, PL0_ASSIGN);

    node->dst = dst;
    node->src = src;

    return node;
}

node_t *pl0_odd(scan_t *scan, where_t where, node_t *expr) {
    node_t *zero = ast_int(scan, where, 0);
    node_t *one = ast_int(scan, where, 1);
    node_t *rem = ast_binary(scan, where, BINARY_REM, expr, one);
    node_t *cmp = ast_binary(scan, where, BINARY_NEQ, rem, zero);

    return cmp;
}

pl0_node_t *pl0_if(scan_t *scan, where_t where, node_t *cond, pl0_node_t *then) {
    pl0_node_t *node = pl0_new(scan, where, PL0_IF);

    node->cond = cond;
    node->then = then;

    return node;
}

pl0_node_t *pl0_while(scan_t *scan, where_t where, node_t *cond, pl0_node_t *then) {
    pl0_node_t *node = pl0_new(scan, where, PL0_WHILE);

    node->cond = cond;
    node->then = then;

    return node;
}
