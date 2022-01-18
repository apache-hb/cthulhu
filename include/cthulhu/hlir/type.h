#pragma once

#include "cthulhu/util/macros.h"

typedef enum {
    TYPE_INTEGER,
} metatype_t;

typedef enum {
    WIDTH_CHAR,
    WIDTH_SHORT,
    WIDTH_INT,
    WIDTH_LONG,
    WIDTH_INTMAX,
    WIDTH_INTPTR
} width_t;

typedef enum {
    SIGN_DEFAULT,
    SIGN_SIGNED,
    SIGN_UNSIGNED,
} sign_t;

typedef struct {
    width_t width;
    sign_t sign;
} int_t;

typedef struct {
    metatype_t type;

    union {
        int_t digit;
    };
} type_t;

const type_t *type_integer(width_t width, sign_t sign) NONULL;
