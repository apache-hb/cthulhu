#pragma once

#include "scan.h"

#include "ctu/util/util.h"

#include <stdbool.h>
#include <gmp.h>

typedef enum {
    AST_DIGIT,
    AST_IDENT,

    AST_UNARY,
    AST_BINARY,
    AST_CALL,

    AST_ASSIGN,
    AST_BRANCH,
    AST_WHILE,
    AST_STMTS,

    AST_VALUE,
    AST_DEFINE,

    AST_MODULE
} ast_t;

typedef enum {
    BINARY_ADD,
    BINARY_SUB,
    BINARY_MUL,
    BINARY_DIV,
    BINARY_REM,

    BINARY_EQ,
    BINARY_NEQ,
    BINARY_GT,
    BINARY_GTE,
    BINARY_LT,
    BINARY_LTE,
} binary_t;

typedef enum {
    UNARY_NEG,
    UNARY_ABS,
} unary_t;

typedef struct {
    /* is lookup case insensitive */
    bool case_sensitive:1;

    /* are declarations order independent */
    bool order_independent:1;
} options_t;

typedef struct node_t {
    ast_t kind;

    scan_t *scan;
    where_t where;

    union {
        /* AST_DIGIT */
        mpz_t digit;

        /* AST_IDENT */
        char *ident;

        /** 
         * AST_UNARY 
         * 
         * unary operand
         */
        struct {
            unary_t unary;
            struct node_t *operand;
        };

        /** 
         * AST_BINARY 
         * 
         * lhs binary rhs
         */
        struct {
            binary_t binary;
            struct node_t *lhs;
            struct node_t *rhs;
        };

        /**
         * AST_CALL
         * 
         * call(args...)
         */
        struct {
            struct node_t *call;
            vector_t *args;
        };

        /**
         * AST_ASSIGN
         * 
         * dst = src;
         */
        struct {
            struct node_t *dst;
            struct node_t *src;
        };

        /**
         * AST_WHILE
         * 
         * while (cond)
         *     then
         * else
         *     other
         * 
         * AST_BRANCH
         * 
         * if (cond)
         *    then
         * else
         *    other
         */
        struct {
            struct node_t *cond;
            struct node_t *then;
            struct node_t *other;
        };

        /**
         * AST_STMTS
         * 
         * {
         *    stmts...
         * }
         */
        vector_t *stmts;

        /* any named declaration */
        struct {
            /* name of the declaration */
            struct node_t *name;

            union {
                /**
                 * AST_VALUE 
                 * 
                 * type name = value;
                 */
                struct {
                    struct node_t *type;
                    struct node_t *value;
                };

                /** 
                 * AST_DEFINE 
                 * 
                 * result name(params...) {
                 *     return body;
                 * }
                 */
                struct {
                    vector_t *params;
                    struct node_t *result;
                    struct node_t *body;
                };
            };
        };

        /** 
         * AST_MODULE 
         * 
         * declarations...
         */
        struct {
            const options_t *options; /* language and module settings */

            vector_t *declarations; /* all declarations */
        };
    };
} node_t;

/* basics */
node_t *ast_digit(scan_t *scan, where_t where, mpz_t digit);
node_t *ast_int(scan_t *scan, where_t where, int digit);
node_t *ast_ident(scan_t *scan, where_t where, char *ident);

/* expressions */
node_t *ast_unary(scan_t *scan, where_t where, unary_t unary, node_t *operand);
node_t *ast_binary(scan_t *scan, where_t where, binary_t binary, node_t *lhs, node_t *rhs);
node_t *ast_call(scan_t *scan, where_t where, node_t *call, vector_t *args);

/* statements */
node_t *ast_assign(scan_t *scan, where_t where, node_t *dst, node_t *src);
node_t *ast_while(scan_t *scan, where_t where, node_t *cond, node_t *then);
node_t *ast_branch(scan_t *scan, where_t where, node_t *cond, node_t *then, node_t *other);
node_t *ast_stmts(scan_t *scan, where_t where, vector_t *stmts);

/* declarations */
node_t *ast_value(scan_t *scan, where_t where, node_t *name, node_t *type, node_t *value);
node_t *ast_define(scan_t *scan, where_t where, node_t *name, 
    vector_t *params, node_t *result, node_t *body
);

node_t *ast_module(scan_t *scan, where_t where, const options_t *options, vector_t *declarations);
