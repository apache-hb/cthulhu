#pragma once

#include "cthulhu/ast/ast.h"

typedef enum {
    DIGIT_CHAR,
    DIGIT_SHORT,
    DIGIT_INT,
    DIGIT_LONG,

    DIGIT_TOTAL
} digit_t;

typedef enum {
    SIGN_SIGNED,
    SIGN_UNSIGNED,
    SIGN_DEFAULT,

    SIGN_TOTAL
} sign_t;

typedef enum {
    // builtin types
    TYPE_INTEGER,
    TYPE_BOOLEAN,
    TYPE_STRING,
    TYPE_VOID,

    // user defined types
    TYPE_SIGNATURE,
    TYPE_POINTER,
    TYPE_ALIAS,

    // error type
    TYPE_ERROR
} metatype_t;

typedef struct type_t {
    metatype_t type; // the type of this type
    const char *name; // the name of this type
    const node_t *node; // where this type was defined, NULL if builtin

    union {
        struct type_t *base;

        struct type_t *alias;

        struct {
            sign_t sign;
            digit_t width;
        };

        struct {
            struct type_t *result;
            vector_t *params;
            bool variadic;
        };
    };
} type_t;

type_t *type_integer(const char *name);
type_t *type_digit(const char *name, const node_t *node, sign_t sign, digit_t width);

type_t *type_boolean(const char *name);
type_t *type_string(const char *name);
type_t *type_void(const char *name);
type_t *type_signature(const char *name, type_t *result, vector_t *params, bool variadic);
type_t *type_pointer(const char *name, type_t *base, const node_t *node);
type_t *type_alias(const char *name, type_t *alias);

type_t *type_error(const char *error, const node_t *node);

bool type_is_integer(const type_t *type);
bool type_is_boolean(const type_t *type);
bool type_is_string(const type_t *type);
bool type_is_void(const type_t *type);
bool type_is_signature(const type_t *type);

const char *type_get_name(const type_t *type);
