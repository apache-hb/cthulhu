#include "cthulhu/data/value.h"

#include "cthulhu/util/util.h"

const type_t *typeof_value(const value_t *value) {
    return value->type;
}

static value_t *value_new(const type_t *type) {
    value_t *value = ctu_malloc(sizeof(value_t));
    value->type = type;
    return value;
}

value_t *value_integer(const type_t *type, int digit) {
    value_t *value = value_new(type);
    mpz_init_set_si(value->integer, digit);
    return value;
}

value_t *value_digit(const type_t *type, mpz_t digit) {
    value_t *value = value_new(type);
    mpz_init_set(value->integer, digit);
    return value;
}

value_t *value_string(const type_t *type, const char *string) {
    value_t *value = value_new(type);
    value->string = string;
    return value;
}

value_t *value_bool(const type_t *type, bool boolean) {
    value_t *value = value_new(type);
    value->boolean = boolean;
    return value;
}
