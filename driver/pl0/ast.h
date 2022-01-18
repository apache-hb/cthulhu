#pragma once

#include "cthulhu/ast/ast.h"
#include "cthulhu/ast/ops.h"

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

    PL0_MODULE
} pl0_type_t;

typedef struct pl0_t {
    pl0_type_t type;
    node_t *node;

    struct {
        /* integer literal */
        mpz_t digit;

        /* an identifier */
        const char *ident;

        /* a procedure to call */
        const char *procedure;

        /* a value to print */
        struct pl0_t *print;

        /* a unary operation */
        struct {
            unary_t unary;
            struct pl0_t *operand;
        };

        /* a binary operation */
        struct {
            binary_t binary;
            struct pl0_t *lhs;
            struct pl0_t *rhs;
        };

        /* an assignment */
        struct {
            const char *dst;
            struct pl0_t *src;
        };

        /* a conditional branch */
        struct {
            struct pl0_t *cond;
            struct pl0_t *then;
        };

        /* statements */
        vector_t *stmts;

        /* a declaration */
        struct {
            const char *name;

            union {
                /* a procedure */
                struct {
                    /* the procedures local variables */
                    vector_t *locals;

                    /* the procedure body */
                    vector_t *body;
                };

                /* a variable */
                struct pl0_t *value;
            };
        };

        struct {
            /* all immutable globals in the program */
            vector_t *consts;

            /* all mutable globals in the program */
            vector_t *variables;

            /* all procedures in the program */
            vector_t *procedures;

            /* the entry point function, if there is one */
            struct pl0_t *entry;

            /* the public name of this module, defaults to the file name */
            const char *mod;
        };
    };
} pl0_t;

pl0_t *pl0_digit(scan_t *scan, where_t where, mpz_t digit);
pl0_t *pl0_ident(scan_t *scan, where_t where, const char *ident);

pl0_t *pl0_binary(scan_t *scan, where_t where, binary_t binary, pl0_t *lhs, pl0_t *rhs);
pl0_t *pl0_unary(scan_t *scan, where_t where, unary_t unary, pl0_t *operand);
pl0_t *pl0_odd(scan_t *scan, where_t where, pl0_t *operand);

pl0_t *pl0_print(scan_t *scan, where_t where, pl0_t *operand);
pl0_t *pl0_assign(scan_t *scan, where_t where, const char *dst, pl0_t *src);
pl0_t *pl0_call(scan_t *scan, where_t where, const char *procedure);

pl0_t *pl0_branch(scan_t *scan, where_t where, pl0_t *cond, pl0_t *then);
pl0_t *pl0_loop(scan_t *scan, where_t where, pl0_t *cond, pl0_t *body);

pl0_t *pl0_stmts(scan_t *scan, where_t where, vector_t *stmts);

pl0_t *pl0_procedure(scan_t *scan, where_t where, const char *name, vector_t *locals, vector_t *body);
pl0_t *pl0_value(scan_t *scan, where_t where, const char *name, pl0_t *value);

pl0_t *pl0_module(scan_t *scan, where_t where, const char *mod, vector_t *consts, vector_t *variables, vector_t *procedures, pl0_t *entry);

