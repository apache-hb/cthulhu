#include "type.h"

#include "ctu/util/str.h"

#include <stdint.h>

bool is_const(const type_t *type) {
    return !type->mut;
}

bool is_any(const type_t *type) {
    return type->type == TY_ANY;
}

bool is_literal(const type_t *type) {
    return type->type == TY_LITERAL_INTEGER;
}

bool is_integer(const type_t *type) {
    return is_digit(type) || is_literal(type);
}

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

bool is_void(const type_t *type) {
    return type->type == TY_VOID;
}

bool is_poison(const type_t *type) {
    return type->type == TY_POISON;
}

bool is_pointer(const type_t *type) {
    return type->type == TY_PTR;
}

bool is_closure(const type_t *type) {
    return type->type == TY_CLOSURE;
}

bool is_varargs(const type_t *type) {
    return type->type == TY_VARARGS;
}

bool is_variadic(const type_t *type) {
    if (!is_closure(type)) {
        return false;
    }

    if (vector_len(type->args) == 0) {
        return false;
    }

    return is_varargs(vector_tail(type->args));
}

size_t minimum_params(const type_t *type) {
    if (!is_closure(type)) {
        return SIZE_MAX;
    }

    if (is_variadic(type)) {
        return vector_len(type->args) - 1;
    }

    return vector_len(type->args);
}

const type_t *closure_result(const type_t *type) {
    if (!is_closure(type)) {
        return type_poison("non closure types do not return");
    }

    return type->result;
}

const type_t *param_at(const type_t *type, size_t idx) {
    if (!is_closure(type)) {
        return type_poison("non closure types do not accept parameters");
    }

    if (is_variadic(type)) {
        return vector_len(type->args) - 1 > idx
            ? vector_get(type->args, idx)
            : type_any();
    }

    return vector_len(type->args) > idx
        ? vector_get(type->args, idx)
        : type_poison("parameter out of range");
}

bool is_string(const type_t *type) {
    return type->type == TY_STRING;
}

const type_t *types_common(const type_t *lhs, const type_t *rhs) {
    if (is_digit(lhs) && is_digit(rhs)) {
        digit_t ld = lhs->digit;
        digit_t rd = rhs->digit;

        bool sign = ld.sign || rd.sign;
        int_t kind = MAX(ld.kind, rd.kind);

        return type_digit(sign, kind);
    }

    if (is_poison(lhs) || is_poison(rhs)) {
        char *err = format("poisoned common type (%s, %s)", type_format(lhs), type_format(rhs));
        return type_poison(err);
    }

    if (is_any(lhs)) { return rhs; }
    if (is_any(rhs)) { return lhs; }

    if (is_literal(lhs) && is_literal(rhs)) {
        return type_literal_integer();
    }

    if (is_literal(lhs) && is_digit(rhs)) {
        return rhs;
    }

    if (is_digit(lhs) && is_literal(rhs)) {
        return lhs;
    }

    if (is_bool(lhs) && is_bool(rhs)) {
        return type_bool();
    }

    char *err = format("cannot get common type of unrelated types (%s, %s)", type_format(lhs), type_format(rhs));
    return type_poison(err);
}
