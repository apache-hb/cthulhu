#pragma once

#include "type.h"

#include <gmp.h>

typedef struct value_t {
    const type_t *type;

    union {
        mpz_t integer;
        bool boolean;
        const char *string;
    };
} value_t;

const type_t *typeof_value(const value_t *value);

/**
 * @brief create a digit literal from a gmp integer
 * 
 * @param type the type of the literal
 * @param digit the gmp integer
 * @return value_t* the new literal
 */
value_t *value_digit(const type_t *type, mpz_t digit);

/**
 * @brief create a digit literal from an integer.
 * 
 * @param type the type of this integer
 * @param digit the integer value
 * @return value_t* the new literal
 */
value_t *value_integer(const type_t *type, int digit);

value_t *value_string(const type_t *type, const char *string);
