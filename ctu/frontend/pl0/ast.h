#pragma once

#include "ctu/ast/scan.h"
#include "ctu/util/util.h"

#include <gmp.h>

typedef enum {
    PL0_PROGRAM,
    PL0_CONST
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
        };

        /* PL0_CONST */
        struct {
            char *key;
            mpz_t value;
        };
    };
} pl0_node_t;

pl0_node_t *pl0_program(scan_t *scan, where_t where, vector_t *consts);
pl0_node_t *pl0_const(scan_t *scan, where_t where, char *key, mpz_t value);
