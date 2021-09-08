#pragma once

#include "ctu/ast/ast.h"

#include <gmp.h>

typedef enum {
    CTU_DIGIT,
    CTU_IDENT,

    CTU_VALUE,

    CTU_MODULE
} ctu_type_t;

typedef struct ctu_t {
    ctu_type_t type;
    node_t *node;

    union {
        mpz_t digit;

        const char *ident;

        struct {
            const char *name;

            union {
                struct ctu_t *value;
            };
        };

        vector_t *decls;
    };
} ctu_t;

ctu_t *ctu_digit(scan_t *scan, where_t where, mpz_t digit);
ctu_t *ctu_ident(scan_t *scan, where_t where, const char *ident);

ctu_t *ctu_value(scan_t *scan, where_t where, const char *name, ctu_t *value);

ctu_t *ctu_module(scan_t *scan, where_t where, vector_t *decls);
