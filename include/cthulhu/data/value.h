#pragma once

#include <gmp.h>

#include "type.h"

typedef struct value_t {
    const type_t *type; // type of the value
    const node_t *node; // where this value was created

    union {
        mpz_t digit;
        const char *string;
    };
} value_t;

value_t *value_string(const type_t *type, const node_t *node, const char *str) NONULL;
value_t *value_int(const type_t *type, const node_t *node, int digit) NONULL;
value_t *value_digit(const type_t *type, const node_t *node, const mpz_t digit) NONULL;
