#pragma once

#include "scan.h"

#include "ctu/util/util.h"

#include <stdbool.h>
#include <gmp.h>

typedef enum {
    AST_DIGIT,
    AST_IDENT

} ast_t;

typedef struct node_t {
    ast_t kind;

    scan_t *scan;
    where_t where;

    union {
        /* AST_DIGIT */
        mpz_t digit;

        /* AST_IDENT */
        char *ident;
    };
} node_t;

node_t *ast_digit(scan_t *scan, where_t where, mpz_t digit);
node_t *ast_ident(scan_t *scan, where_t where, char *ident);
