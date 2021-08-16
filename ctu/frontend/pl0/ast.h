#pragma once

#include "ctu/ast/ast.h"
#include "ctu/ast/scan.h"
#include "ctu/util/util.h"

#include <gmp.h>

typedef enum {
    /* an entire program */
    PL0_PROGRAM,

    /* named declarations */
    PL0_VALUE,
    PL0_GLOBAL,
    PL0_PROCEDURE,

    /* statements */
    PL0_STATEMENTS,
    PL0_CALL,
    PL0_ASSIGN,

    PL0_IF,
    PL0_WHILE
} pl0_ast_t;

typedef struct pl0_node_t {
    pl0_ast_t type;

    scan_t *scan;
    where_t where;

    union {
        /* PL0_PROGRAM */
        struct {
            /* constant values */
            vector_t *consts;

            /* global values */
            vector_t *vars;

            /* function decls */
            vector_t *procedures;
        };

        /* PL0_VALUE */
        struct {
            node_t *name;

            union {
                /* PL0_CONST */
                node_t *value;

                /* PL0_PROCEDURE */
                struct {
                    vector_t *locals;
                };
            };
        };

        /* PL0_STATEMENTS */
        vector_t *statements;

        /* PL0_CALL */
        node_t *call;

        /* PL0_ASSIGN */
        struct {
            node_t *dst;
            node_t *src;
        };

        /* PL0_IF/PL0_WHILE */
        struct {
            node_t *cond;
            struct pl0_node_t *then;
        };
    };
} pl0_node_t;

pl0_node_t *pl0_program(
    scan_t *scan, where_t where, 
    vector_t *consts, vector_t *vars, vector_t *procedures
);

pl0_node_t *pl0_value(scan_t *scan, where_t where, node_t *name);
pl0_node_t *pl0_const(scan_t *scan, where_t where, node_t *name, node_t *value);
pl0_node_t *pl0_procedure(scan_t *scan, where_t where, node_t *name, vector_t *locals);

pl0_node_t *pl0_statements(scan_t *scan, where_t where, vector_t *statements);
pl0_node_t *pl0_call(scan_t *scan, where_t where, node_t *call);
pl0_node_t *pl0_assign(scan_t *scan, where_t where, node_t *dst, node_t *src);

pl0_node_t *pl0_if(scan_t *scan, where_t where, node_t *cond, pl0_node_t *then);
pl0_node_t *pl0_while(scan_t *scan, where_t where, node_t *cond, pl0_node_t *then);

node_t *pl0_odd(scan_t *scan, where_t where, node_t *expr);
