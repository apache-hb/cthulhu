#pragma once

#include "scan.h"
#include <gmp.h>

#include "cthulhu/data/type.h"

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

const char *get_name_for_sign(sign_t sign);
const char *get_name_for_inttype(digit_t digit);
const char *get_name_for_digit(sign_t sign, digit_t digit);
