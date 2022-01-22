#pragma once

typedef enum {
    VALUE_TYPE // the type of all types
} value_type_t;

/**
 * @brief all values are also types
 */
typedef struct value_t {
    const value_t *type; // the type of this value
    const char *name; // the name of this value or type

    union {
        mpz_t digit; // an integer value
    };
} value_t;
