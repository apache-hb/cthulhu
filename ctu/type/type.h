#pragma once

#include <stdbool.h>

#include "ctu/util/util.h"

typedef enum {
    TY_VOID,
    TY_DIGIT,
    TY_BOOL,
    TY_CLOSURE,
    TY_POISON
} metatype_t;

typedef enum {
    TY_CHAR,
    TY_SHORT,
    TY_INT,
    TY_LONG,
    TY_SIZE,
    TY_INTPTR,
    TY_INTMAX
} int_t;

typedef enum {
    SIGNED,
    UNSIGNED
} sign_t;

typedef struct {
    sign_t sign;
    int_t kind;
} digit_t;

typedef struct type_t {
    metatype_t type;

    /* is this type mutable */
    bool mut;

    union {
        digit_t digit;

        struct {
            vector_t *args;
            struct type_t *result;
        };

        const char *msg;
    };
} type_t;

char *type_format(const type_t *type);

type_t *type_digit(sign_t sign, int_t kind);
type_t *type_void(void);
type_t *type_closure(vector_t *args, type_t *result);
type_t *type_bool(void);
type_t *type_poison(const char *msg);

void type_mut(type_t *type, bool mut);

bool is_digit(const type_t *type);
bool is_bool(const type_t *type);

bool is_signed(const type_t *type);
bool is_unsigned(const type_t *type);

typedef enum {
    INTEXACT, /* exact integer match */
    INTWIDTH, /* width match */
    INTSIGN, /* sign match */

    BOOLEXACT, /* exact boolean match */

    NONE, /* no similarity */
} typecmp_t;

typecmp_t types_equal(const type_t *lhs, const type_t *rhs);
type_t *types_common(const type_t *lhs, const type_t *rhs);
