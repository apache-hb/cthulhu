#include "cthulhu/hlir/type.h"

static type_t *new_type(const node_t *node, const char *name, metatype_t type) {
    type_t *result = ctu_malloc(sizeof(type_t));
    result->node = node;
    result->name = name;
    result->type = type;
    return result;
}

type_t *type_integer(const node_t *node, const char *name, width_t width, sign_t sign) {
    int_t digit = { width, sign };
    type_t *result = new_type(node, name, TYPE_INTEGER);
    result->digit = digit;
    return result;
}

type_t *type_string(const node_t *node, const char *name, encoding_t encoding) {
    type_t *result = new_type(node, name, TYPE_STRING);
    result->encoding = encoding;
    return result;
}

type_t *type_closure(const node_t *node, const char *name, type_t *result, vector_t *params, bool variadic) {
    closure_t closure = { result, params, variadic };
    type_t *type = new_type(node, name, TYPE_CLOSURE);
    type->closure = closure;
    return type;
}

static bool type_is(const type_t *type, metatype_t of) {
    return type->type == of;
}

bool is_integer(const type_t *type) {
    return type_is(type, TYPE_INTEGER);
}

bool is_string(const type_t *type) {
    return type_is(type, TYPE_STRING);
}

bool is_closure(const type_t *type) {
    return type_is(type, TYPE_CLOSURE);
}

bool is_variadic(const type_t *type) {
    return is_closure(type) && type->closure.variadic;
}