#include "type.h"

bool is_digit(const type_t *type) {
    return type->type == TY_INTEGER;
}

bool is_bool(const type_t *type) {
    return type->type == TY_BOOL;
}

bool is_signed(const type_t *type) {
    if (!is_digit(type)) {
        return false;
    }

    return type->digit.sign == SIGNED;
}

bool is_unsigned(const type_t *type) {
    if (!is_digit(type)) {
        return false;
    }

    return type->digit.sign == UNSIGNED;
}

type_t *types_common(const type_t *lhs, const type_t *rhs) {
    if (is_digit(lhs) && is_digit(rhs)) {
        digit_t ld = lhs->digit;
        digit_t rd = rhs->digit;

        bool sign = ld.sign || rd.sign;
        int_t kind = MAX(ld.kind, rd.kind);

        return type_digit(sign, kind);
    }

    if (is_bool(lhs) && is_bool(rhs)) {
        return type_bool();
    }

    return type_poison("cannot get common type of unrelated types");
}
