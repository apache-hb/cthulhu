#include "cthulhu/ssa/value.h"

static value_t *value_new(const type_t *type, const node_t *node) {
    value_t *value = ctu_malloc(sizeof(value_t));
    value->type = type;
    value->node = node;
    return value;
}

value_t *value_digit(const type_t *type, const node_t *node, const mpz_t digit) {
    value_t *value = value_new(type, node);
    mpz_init_set(value->digit, digit);
    return value;
}

value_t *value_int(const type_t *type, const node_t *node, int64_t digit) {
    value_t *value = value_new(type, node);
    mpz_init_set_si(value->digit, digit);
    return value;
}

value_t *value_str(const type_t *type, const node_t *node, const char *str) {
    value_t *value = value_new(type, node);
    value->str = str;
    return value;
}
