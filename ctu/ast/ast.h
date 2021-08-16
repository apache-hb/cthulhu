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

    AST_VALUE
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

typedef struct node_t {
    ast_t kind;

    scan_t *scan;
    where_t where;

    union {
        struct {
            unary_t unary;
            struct node_t *operand;
        };

        struct {
            binary_t binary;
            struct node_t *lhs;
            struct node_t *rhs;
        };

        struct {
            struct node_t *name;

            union {
                struct node_t *value;
            };
        };

        /* AST_DIGIT */
        mpz_t digit;

        /* AST_IDENT */
        char *ident;
    };
} node_t;

node_t *ast_digit(scan_t *scan, where_t where, mpz_t digit);
node_t *ast_int(scan_t *scan, where_t where, int digit);
node_t *ast_ident(scan_t *scan, where_t where, char *ident);

node_t *ast_unary(scan_t *scan, where_t where, unary_t unary, node_t *operand);
node_t *ast_binary(scan_t *scan, where_t where, binary_t binary, node_t *lhs, node_t *rhs);
