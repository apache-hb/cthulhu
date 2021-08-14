#pragma once

#include "scan.h"

#include "ctu/util/util.h"

#include <stdbool.h>
#include <gmp.h>

typedef enum {
    AST_DIGIT,

    AST_DECL_VALUE,

    AST_PROGRAM
} ast_t;

typedef struct node_t {
    ast_t kind;

    scan_t *scan;
    where_t where;

    union {
        /* AST_DIGIT */
        mpz_t digit;

        /* AST_DECL_VALUE */
        struct {
            bool mutable;
            char *name;
            struct node_t *value;
        };

        /* AST_PROGRAM */
        struct {
            vector_t *decls;
        };
    };
} node_t;

node_t *ast_digit(scan_t *scan, where_t where, mpz_t digit);
node_t *ast_digit_zero(scan_t *scan, where_t where);
node_t *ast_value(scan_t *scan, where_t where, bool mutable, char *name, node_t *value);

node_t *ast_program(scan_t *scan, where_t where, vector_t *decls);
