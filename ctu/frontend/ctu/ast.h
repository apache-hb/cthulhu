#pragma once

#include "ctu/ast/ast.h"
#include "ctu/ast/ops/ops.h"
#include "ctu/lir/lir.h"

#include <gmp.h>

typedef enum {
    CTU_DIGIT,
    CTU_IDENT,

    CTU_UNARY,
    CTU_BINARY,

    CTU_STMTS,

    CTU_TYPENAME,
    CTU_POINTER,

    CTU_VALUE,
    CTU_PARAM,
    CTU_DEFINE,

    CTU_MODULE
} ctu_type_t;

typedef struct ctu_t {
    ctu_type_t type;
    node_t *node;
    lir_t *lir;

    union {
        mpz_t digit;

        const char *ident;

        struct ctu_t *ptr;

        struct {
            unary_t unary;
            struct ctu_t *operand;
        };

        struct {
            binary_t binary;
            struct ctu_t *lhs;
            struct ctu_t *rhs;
        };

        struct {
            const char *name;

            union {
                struct {
                    struct ctu_t *kind;
                    struct ctu_t *value;
                };

                struct {
                    vector_t *params;
                    struct ctu_t *result;
                    struct ctu_t *body;
                };
            };
        };

        vector_t *decls;

        vector_t *stmts;
    };
} ctu_t;

/* literals */
ctu_t *ctu_digit(scan_t *scan, where_t where, mpz_t digit);
ctu_t *ctu_ident(scan_t *scan, where_t where, const char *ident);

/* expressions */
ctu_t *ctu_unary(scan_t *scan, where_t where, unary_t unary, ctu_t *operand);
ctu_t *ctu_binary(scan_t *scan, where_t where, binary_t binary, ctu_t *lhs, ctu_t *rhs);

/* statements */
ctu_t *ctu_stmts(scan_t *scan, where_t where, vector_t *stmts);

/* types */
ctu_t *ctu_pointer(scan_t *scan, where_t where, 
                   ctu_t *ptr);
ctu_t *ctu_typename(scan_t *scan, where_t where, 
                    const char *name);

/* declarations */
ctu_t *ctu_value(scan_t *scan, where_t where, 
                 const char *name, ctu_t *type, 
                 ctu_t *value);

ctu_t *ctu_param(scan_t *scan, where_t where,
                 const char *name, ctu_t *type);

ctu_t *ctu_define(scan_t *scan, where_t where, 
                  const char *name, vector_t *params, 
                  ctu_t *result, ctu_t *body);

/* modules */
ctu_t *ctu_module(scan_t *scan, where_t where, 
                  vector_t *decls);
