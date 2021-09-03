#pragma once

#include <stdbool.h>

#include "ctu/util/util.h"

typedef enum {
    TY_VOID,
    TY_DIGIT,
    TY_CLOSURE
} metatype_t;

typedef enum {
    TY_CHAR,
    TY_SHORT,
    TY_INT,
    TY_LONG,
    TY_SIZE
} int_t;

typedef struct {
    bool sign;
    int_t kind;
} digit_t;

typedef struct type_t {
    metatype_t type;

    union {
        digit_t digit;

        struct {
            vector_t *args;
            struct type_t *result;
        };
    };
} type_t;

type_t *type_digit(bool sign, int_t kind);
type_t *type_closure(vector_t *args, type_t *result);
char *type_format(const type_t *type);
