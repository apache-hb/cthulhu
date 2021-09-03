#pragma once

#include <stdbool.h>

typedef enum {
    TY_VOID,
    TY_DIGIT
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

typedef struct {
    metatype_t type;

    union {
        digit_t digit;
    };
} type_t;

type_t *type_digit(bool sign, int_t kind);
char *type_format(const type_t *type);
