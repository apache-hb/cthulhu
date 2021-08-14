#pragma once

#include "ctu/ast/scan.h"
#include "ctu/util/util.h"

#include <gmp.h>

typedef enum {
    PL0_PROGRAM,

    PL0_CONST,
    PL0_GLOBAL,

    PL0_IDENT,
    PL0_NUMBER
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
        };

        /* PL0_GLOBAL */
        struct {
            struct pl0_node_t *key;
            struct pl0_node_t *value;
        };

        /* PL0_IDENT */
        char *ident;

        /* PL0_NUMBER */
        mpz_t number;
    };
} pl0_node_t;

pl0_node_t *pl0_program(
    scan_t *scan, where_t where, 
    vector_t *consts, vector_t *vars
);

pl0_node_t *pl0_global(scan_t *scan, where_t where, pl0_node_t *key, pl0_node_t *value);
pl0_node_t *pl0_const(scan_t *scan, where_t where, pl0_node_t *key, pl0_node_t *value);

pl0_node_t *pl0_ident(scan_t *scan, where_t where, char *ident);
pl0_node_t *pl0_number(scan_t *scan, where_t where, mpz_t number);
