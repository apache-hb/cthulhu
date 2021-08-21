#include "ast.h"

node_t *pl0_odd(scan_t *scan, where_t where, node_t *expr) {
    node_t *zero = ast_int(scan, where, 0);
    node_t *one = ast_int(scan, where, 1);
    node_t *rem = ast_binary(scan, where, BINARY_REM, expr, one);
    node_t *cmp = ast_binary(scan, where, BINARY_NEQ, rem, zero);

    return cmp;
}

node_t *pl0_module(scan_t *scan, where_t where, vector_t *consts, vector_t *values, vector_t *procs, node_t *body) {
    vector_t *vars = vector_join(consts, values);
    
    /* a module might not have any toplevel statements */
    if (body) {
        node_t *name = ast_ident(scan, body->where, ctu_strdup(scan->path));
        node_t *toplevel = ast_define(body->scan, body->where, name, 
            /* locals = */ vector_new(0), 
            /* params = */ vector_new(0), 
            /* result = */ NULL, 
            /* body = */ body
        );

        vector_push(&procs, toplevel);
    }

    return ast_module(scan, where, vars, procs);
}
