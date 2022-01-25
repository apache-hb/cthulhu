#pragma once

typedef enum {
    TYPE_INTEGER,
    TYPE_BOOLEAN,
    TYPE_STRING,
    TYPE_ERROR
} metatype_t;

typedef struct type_t {
    metatype_t type;
    const char *name;
} type_t;

type_t *type_integer(const char *name);
type_t *type_boolean(const char *name);
type_t *type_string(const char *name);
type_t *type_error(const char *error);
