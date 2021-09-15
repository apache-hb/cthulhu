#include "value.h"

value_t *value_of(const type_t *type) {
    value_t *value = NEW(value_t);
    value->type = type;
    return value;
}

value_t *value_poison(const char *msg) {
    return value_of(type_poison(msg));
}

value_t *value_digit(const type_t *type, mpz_t digit) {
    value_t *value = value_of(type);
    mpz_init_set(value->digit, digit);
    return value;
}

value_t *value_int(const type_t *type, int digit) {
    value_t *value = value_of(type);
    mpz_init_set_si(value->digit, digit);
    return value;
}

value_t *value_ptr(const type_t *type, value_t *ptr) {
    value_t *value = value_of(type);
    value->ptr = ptr;
    return value;
}
