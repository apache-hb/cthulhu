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

pl0_node_t *pl0_global(scan_t *scan, where_t where, node_t *name) {
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
