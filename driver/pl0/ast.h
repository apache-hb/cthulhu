#pragma once

#include "cthulhu/hlir/ops.h"
#include "scan/node.h"
#include "std/vector.h"

#include <gmp.h>

typedef enum
{
    ePl0Digit,
    ePl0Ident,

    ePl0Odd,
    ePl0Unary,
    ePl0Binary,
    ePl0Compare,

    ePl0Assign,
    ePl0Call,
    ePl0Branch,
    ePl0Loop,
    ePl0Print,
    ePl0Stmts,

    ePl0Value,
    ePl0Procedure,

    ePl0Module
} pl0_type_t;

typedef struct pl0_t
{
    pl0_type_t type;
    node_t node;

    union {
        /* integer literal */
        mpz_t digit;

        /* an identifier */
        const char *ident;

        /* a procedure to call */
        const char *procedure;

        /* a value to print */
        struct pl0_t *print;

        /* a unary operation */
        struct
        {
            unary_t unary;
            struct pl0_t *operand;
        };

        /* a binary or compare operation */
        struct
        {
            struct pl0_t *lhs;
            struct pl0_t *rhs;

            union {
                binary_t binary;
                compare_t compare;
            };
        };

        /* an assignment */
        struct
        {
            const char *dst;
            struct pl0_t *src;
        };

        /* a conditional branch */
        struct
        {
            struct pl0_t *cond;
            struct pl0_t *then;
        };

        /* statements */
        vector_t *stmts;

        /* a declaration */
        struct
        {
            const char *name;

            union {
                /* a procedure */
                struct
                {
                    /* the procedures local variables */
                    vector_t *locals;

                    /* the procedure body */
                    vector_t *body;
                };

                /* a variable */
                struct pl0_t *value;
            };
        };

        struct
        {
            /* all immutable globals in the program */
            vector_t *consts;

            /* all mutable globals in the program */
            vector_t *globals;

            /* all procedures in the program */
            vector_t *procs;

            /* the entry point function, if there is one */
            struct pl0_t *entry;

            /* the public name of this module, defaults to the file name */
            const char *mod;

            /* all modules imported by this module */
            vector_t *imports;
        };
    };
} pl0_t;

pl0_t *pl0_digit(scan_t *scan, where_t where, mpz_t digit);
pl0_t *pl0_ident(scan_t *scan, where_t where, const char *ident);

pl0_t *pl0_binary(scan_t *scan, where_t where, binary_t binary, pl0_t *lhs, pl0_t *rhs);
pl0_t *pl0_compare(scan_t *scan, where_t where, compare_t compare, pl0_t *lhs, pl0_t *rhs);

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

pl0_t *pl0_module(scan_t *scan, where_t where, const char *mod, vector_t *imports, vector_t *consts, vector_t *globals,
                  vector_t *procs, pl0_t *entry);
