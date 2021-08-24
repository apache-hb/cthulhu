#include "ast.h"

static const options_t OPTIONS = {
    .case_sensitive = false,
    .order_independent = true
};

static node_t *pl0_int(void) {
    type_t *type = type_digit(true, TY_INT);
    node_t *node = ast_type(NULL, nowhere, type);
    return node;
}

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
            /* params = */ vector_new(0), 
            /* result = */ pl0_int(), 
            /* body = */ body
        );

        vector_push(&procs, toplevel);
    }

    vector_t *decls = vector_join(vars, procs);

    /* these have been copied out */
    vector_delete(vars);
    vector_delete(procs);
    vector_delete(consts);
    vector_delete(values);

    return ast_module(scan, where, &OPTIONS, decls);
}

node_t *pl0_procedure(scan_t *scan, where_t where, node_t *name, vector_t *locals, node_t *body) {
    vector_t *stmts = vector_join(locals, body->stmts);

    return ast_define(scan, where, name, 
        /* params = */ vector_new(0), 
        /* result = */ pl0_int(), 
        /* body = */ ast_stmts(scan, where, stmts)
    );
}
