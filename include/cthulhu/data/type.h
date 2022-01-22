#pragma once

#include "cthulhu/ast/ast.h"

typedef enum {
    TYPE_INTEGER,
    TYPE_STRING,
    TYPE_CLOSURE,
    TYPE_VOID,
    TYPE_BOOLEAN
} typeof_t;

typedef enum {
    WIDTH_CHAR, // at least 8 bits
    WIDTH_SHORT, // at least 16 bits
    WIDTH_INT, // at least 32 bits
    WIDTH_LONG, // at least 64 bits
    WIDTH_SIZE, // the maximum size of a type
    WIDTH_MAX, // the largest native integer size
    WIDTH_PTR, // the size of a pointer
    WIDTH_VAR  // dynamic length, used for compile time values
} width_t;

typedef enum {
    SIGN_SIGNED,
    SIGN_UNSIGNED
} sign_t;

typedef struct type_t {
    typeof_t type;
    const char *name; // the name of the type
    const node_t *node; // where the type was defined

    union {
        struct {
            width_t width;
            sign_t sign;
        };

        struct {
            vector_t *params;
            struct type_t *result;
            bool variadic;
        };
    };
} type_t;

type_t *type_boolean(const node_t *node, const char *name) NONULL;
type_t *type_void(const node_t *node, const char *name) NONULL;
type_t *type_integer(const node_t *node, const char *name, width_t width, sign_t sign) NONULL;
type_t *type_string(const node_t *node, const char *name) NONULL;
type_t *type_closure(const node_t *node, const char *name, vector_t *params, type_t *result, bool variadic) NONULL;

bool type_is(const type_t *type, typeof_t of) NONULL;

bool is_string(const type_t *type) NONULL;
bool is_integer(const type_t *type) NONULL;
bool is_closure(const type_t *type) NONULL;
bool is_variadic(const type_t *type) NONULL;
