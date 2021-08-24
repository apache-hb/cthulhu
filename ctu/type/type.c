#include "type.h"

#include "ctu/util/util.h"

static type_t *type_new(metatype_t meta) {
    type_t *type = ctu_malloc(sizeof(type_t));
    type->type = meta;
    return type;
}

type_t *type_digit(bool sign, int_t kind) {
    type_t *type = type_new(TY_DIGIT);
    digit_t digit = { sign, kind };
    type->digit = digit;
    return type;
}
