#include "cthulhu/data/type.h"

#include "cthulhu/util/util.h"

static type_t *type_new(metatype_t meta, const char *name, const node_t *node) {
    type_t *type = ctu_malloc(sizeof(type_t));
    type->type = meta;
    type->name = name;
    type->node = node;
    return type;
}

type_t *type_integer(const char *name) {
    return type_new(TYPE_INTEGER, name, NULL);
}

type_t *type_boolean(const char *name) {
    return type_new(TYPE_BOOLEAN, name, NULL);
}

type_t *type_string(const char *name) {
    return type_new(TYPE_STRING, name, NULL);
}

type_t *type_void(const char *name) {
    return type_new(TYPE_VOID, name, NULL);
}

type_t *type_signature(const char *name, type_t *result, vector_t *params) {
    type_t *type = type_new(TYPE_SIGNATURE, name, NULL);
    type->result = result;
    type->params = params;
    return type;
}

type_t *type_error(const char *error, const node_t *node) {
    return type_new(TYPE_ERROR, error, node);
}

static bool type_is(const type_t *type, metatype_t meta) {
    return type->type == meta;
}

bool type_is_integer(const type_t *type) {
    return type_is(type, TYPE_INTEGER);
}

bool type_is_boolean(const type_t *type) {
    return type_is(type, TYPE_BOOLEAN);
}

bool type_is_string(const type_t *type) {
    return type_is(type, TYPE_STRING);
}

bool type_is_void(const type_t *type) {
    return type_is(type, TYPE_VOID);
}

bool type_is_signature(const type_t *type) {
    return type_is(type, TYPE_SIGNATURE);
}

const char *type_get_name(const type_t *type) {
    return type->name;
}
