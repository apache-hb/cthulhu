#pragma once

#include "scan.h"

#include <gmp.h>

typedef enum {
    AST_DIGIT,
    
    AST_PROGRAM,

    AST_DECL_VALUE
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
            char *name;
            struct node_t *value;
        };
    };
} node_t;

node_t *ast_digit(scan_t *scan, where_t where, mpz_t digit);
node_t *ast_value(scan_t *scan, where_t where, char *name, node_t *value);
