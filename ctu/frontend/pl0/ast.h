#pragma once

#include "ctu/ast/ast.h"
#include "ctu/ast/scan.h"
#include "ctu/util/util.h"

#include <gmp.h>

typedef enum {
    PL0_DIGIT,
    PL0_IDENT,

    PL0_ODD,
    PL0_UNARY,
    PL0_BINARY,

    PL0_ASSIGN,
    PL0_CALL,
    PL0_BRANCH,
    PL0_LOOP,
    PL0_PRINT,
    PL0_STMTS,

    PL0_VALUE,
    PL0_PROCEDURE,

    PL0_MODULE,
} pl0_type_t;

typedef struct pl0_t {
    pl0_type_t type;
    node_t *node;

    union {
        mpz_t digit;

        const char *ident;

        struct {
            binary_t binary;
            struct pl0_t *lhs;
            struct pl0_t *rhs;
        };

        struct {
            unary_t unary;
            struct pl0_t *operand;
        };

        struct {
            const char *dst;
            struct pl0_t *src;
        };

        struct {
            struct pl0_t *cond;
            struct pl0_t *then;
        };

        vector_t *stmts;

        struct {
            const char *name;

            union {
                struct {
                    vector_t *locals;
                    struct pl0_t *body;
                };

                struct pl0_t *value;
            };
        };

        /**
         * PL0_MODULE
         * 
         * const consts...;
         * var vars...;
         * 
         * procedure...;
         * 
         * toplevel?
         * .
         */
        struct {
            vector_t *consts;
            vector_t *globals;
            vector_t *procs;
            struct pl0_t *toplevel;  
        };
    };
} pl0_t;

pl0_t *pl0_digit(scan_t *scan, where_t where, mpz_t value);

pl0_t *pl0_ident(scan_t *scan, where_t where, const char *name);

pl0_t *pl0_binary(scan_t *scan, where_t where, 
                  binary_t binary, pl0_t *left, pl0_t *right);

pl0_t *pl0_unary(scan_t *scan, where_t where,
                 unary_t unary, pl0_t *operand);

pl0_t *pl0_odd(scan_t *scan, where_t where, pl0_t *operand);

pl0_t *pl0_value(scan_t *scan, where_t where, 
                 const char *name, pl0_t *value);

pl0_t *pl0_assign(scan_t *scan, where_t where,
                  const char *dst, pl0_t *src);

pl0_t *pl0_call(scan_t *scan, where_t where, const char *proc);

pl0_t *pl0_branch(scan_t *scan, where_t where,
                  pl0_t *cond, pl0_t *then);

pl0_t *pl0_loop(scan_t *scan, where_t where,
                 pl0_t *cond, pl0_t *body);

pl0_t *pl0_print(scan_t *scan, where_t where,
                 pl0_t *value);

pl0_t *pl0_stmts(scan_t *scan, where_t where,
                 vector_t *stmts);

pl0_t *pl0_procedure(scan_t *scan, where_t where,
                     const char *name, vector_t *locals,
                     pl0_t *body);

pl0_t *pl0_module(scan_t *scan, where_t where, 
                  vector_t *consts, vector_t *globals, 
                  vector_t *procs, struct pl0_t *toplevel);

#if 0
pl0_t *pl0_odd(scan_t *scan, where_t where, pl0_t *expr);
pl0_t *pl0_print(scan_t *scan, where_t where, pl0_t *expr);
pl0_t *pl0_module(scan_t *scan, where_t where, vector_t *consts, vector_t *values, vector_t *procs, pl0_t *body);
pl0_t *pl0_value(scan_t *scan, where_t where, pl0_t *name, pl0_t *expr);
pl0_t *pl0_procedure(scan_t *scan, where_t where, pl0_t *name, vector_t *locals, pl0_t *body);
#endif