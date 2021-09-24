#pragma once

#include "ctu/type/type.h"
#include "ctu/ast/ast.h"

#include <gmp.h>

typedef struct value_t {
    const type_t *type;

    union {
        mpz_t digit;
        bool boolean;
        const char *string;
        struct value_t *ptr;
    };
} value_t;

value_t *value_of(const type_t *type);
value_t *value_poison(const char *msg);
value_t *value_digit(const type_t *type, mpz_t digit);
value_t *value_int(const type_t *type, int digit);
value_t *value_ptr(const type_t *type, value_t *ptr);

char *value_format(const value_t *value);

void value_delete(value_t *value);
