#pragma once

#include "cthulhu/util/macros.h"
#include "cthulhu/ast/ast.h"

typedef enum {
    TYPE_INTEGER,
} metatype_t;

typedef enum {
    WIDTH_CHAR, // at least 8 bits
    WIDTH_SHORT, // at least 16 bits
    WIDTH_INT, // at least 32 bits
    WIDTH_LONG, // at least 64 bits
    WIDTH_SIZE, // the maximum size of a type
    WIDTH_INTMAX, // the largest native integer size
    WIDTH_INTPTR, // the size of a pointer
    WIDTH_VARIABLE // dynamic length, used for compile time values
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
    const node_t *node;

    union {
        int_t digit;
    };
} type_t;

type_t *type_integer(const node_t *node, width_t width, sign_t sign) NONULL;
