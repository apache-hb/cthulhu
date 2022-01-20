#include "cthulhu/hlir/type.h"

static type_t *new_type(const node_t *node, metatype_t type) {
    type_t *result = ctu_malloc(sizeof(type_t));
    result->node = node;
    result->type = type;
    return result;
}

type_t *type_integer(const node_t *node, width_t width, sign_t sign) {
    int_t digit = { width, sign };
    type_t *result = new_type(node, TYPE_INTEGER);
    result->digit = digit;
    return result;
}
