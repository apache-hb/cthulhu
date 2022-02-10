#pragma once

#include "scan.h"
#include <gmp.h>

typedef enum {
    AST_DIGIT
} astof_t;

typedef struct {
    astof_t type;
    node_t *node;

    union {
        mpz_t digit;  
    };
} ast_t;

ast_t *ast_digit(scan_t *scan, where_t where, mpz_t digit);
