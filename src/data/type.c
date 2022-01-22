#include "cthulhu/data/type.h"

static type_t *type_new(const node_t *node, const char *name, typeof_t of) {
    CTASSERT(name != NULL, "types cannot have no name");

    type_t *type = ctu_malloc(sizeof(type_t));
    type->node = node;
    type->name = name;
    type->type = of;
    return type;
} 

type_t *type_integer(const node_t *node, const char *name, width_t width, sign_t sign) {
    type_t *type = type_new(node, name, TYPE_INTEGER);
    type->width = width;
    type->sign = sign;
    return type;
}

type_t *type_string(const node_t *node, const char *name) {
    return type_new(node, name, TYPE_STRING);
}

type_t *type_closure(const node_t *node, const char *name, vector_t *params, type_t *result, bool variadic) {
    type_t *type = type_new(node, name, TYPE_CLOSURE);
    type->params = params;
    type->result = result;
    type->variadic = variadic;
    return type;
}

type_t *type_void(const node_t *node, const char *name) {
    return type_new(node, name, TYPE_VOID);
}

bool type_is(const type_t *type, typeof_t of) {
    return type->type == of;
}

bool is_string(const type_t *type) {
    return type_is(type, TYPE_STRING);
}

bool is_integer(const type_t *type) {
    return type_is(type, TYPE_INTEGER);
}

bool is_closure(const type_t *type) {
    return type_is(type, TYPE_CLOSURE);
}

bool is_variadic(const type_t *type) {
    return is_closure(type) && type->variadic;
}
