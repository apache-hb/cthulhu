#include "cthulhu/data/value.h"

static value_t *value_new(const type_t *type, const node_t *node) {
    CTASSERT(type != NULL, "values cannot have no type");
    value_t *value = ctu_malloc(sizeof(value_t));
    value->type = type;
    value->node = node;
    return value;
}

value_t *value_string(const type_t *type, const node_t *node, const char *str) {
    value_t *value = value_new(type, node);
    value->string = str;
    return value;
}

value_t *value_digit(const type_t *type, const node_t *node, const mpz_t digit) {
    value_t *value = value_new(type, node);
    mpz_init_set(value->digit, digit);
    return value;
}
