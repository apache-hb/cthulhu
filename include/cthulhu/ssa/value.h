#pragma once

#include "cthulhu/hlir/type.h"

#include <gmp.h>

typedef struct {
    const type_t *type;
    const node_t *node;

    union {
        mpz_t digit;
        const char *str;
    };
} value_t;

value_t *value_digit(const type_t *type, const node_t *node, const mpz_t digit) NONULL;
value_t *value_int(const type_t *type, const node_t *node, int64_t digit) NONULL;
value_t *value_str(const type_t *type, const node_t *node, const char *str) NONULL;
