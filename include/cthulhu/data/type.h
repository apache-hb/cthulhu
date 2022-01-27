#pragma once

#include "cthulhu/ast/ast.h"

typedef enum {
    // builtin types
    TYPE_INTEGER,
    TYPE_BOOLEAN,
    TYPE_STRING,
    TYPE_VOID,

    // user defined types
    TYPE_SIGNATURE,

    // error type
    TYPE_ERROR
} metatype_t;

typedef struct type_t {
    metatype_t type; // the type of this type
    const char *name; // the name of this type
    const node_t *node; // where this type was defined, NULL if builtin

    union {
        struct {
            struct type_t *result;
            vector_t *params;
        };
    };
} type_t;

type_t *type_integer(const char *name);
type_t *type_boolean(const char *name);
type_t *type_string(const char *name);
type_t *type_void(const char *name);
type_t *type_signature(const char *name, type_t *result, vector_t *params);

type_t *type_error(const char *error, const node_t *node);

bool type_is_integer(const type_t *type);
bool type_is_boolean(const type_t *type);
bool type_is_string(const type_t *type);
bool type_is_void(const type_t *type);
bool type_is_signature(const type_t *type);
