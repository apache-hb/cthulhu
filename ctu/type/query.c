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

size_t maximum_params(const type_t *type) {
    if (!is_closure(type)) {
        return SIZE_MAX;
    }

    if (is_variadic(type)) {
        return SIZE_MAX;
    }

    return vector_len(type->args);
}

size_t minimum_params(const type_t *type) {
    if (!is_closure(type)) {
        return SIZE_MAX;
    }

    return vector_len(type->args) - (is_variadic(type) ? 1 : 0);
}

const type_t *closure_result(const type_t *type) {
    if (!is_closure(type)) {
        return type_poison("non closure types do not return");
    }

    return type->result;
}

size_t num_params(const type_t *type) {
    if (!is_closure(type)) {
        return SIZE_MAX;
    }

    return vector_len(type->args);
}

const type_t *param_at(const type_t *type, size_t idx) {
    if (!is_closure(type)) {
        return type_poison_with_node("non closure types do not accept parameters", type->node);
    }

    if (is_variadic(type)) {
        return vector_len(type->args) - 1 > idx
            ? vector_get(type->args, idx)
            : type_any();
    }

    return vector_len(type->args) > idx
        ? vector_get(type->args, idx)
        : type_poison_with_node("parameter out of range", type->node);
}

bool is_string(const type_t *type) {
    return type->type == TY_STRING;
}

bool is_aggregate(const type_t *type) {
    return is_struct(type) || is_union(type);
}

bool is_struct(const type_t *type) {
    return type->type == TY_STRUCT;
}

bool is_union(const type_t *type) {
    return type->type == TY_UNION;
}

size_t field_offset(const type_t *type, const char *name) {
    if (!is_aggregate(type)) {
        return SIZE_MAX;
    }

    size_t len = vector_len(type->fields);
    for (size_t i = 0; i < len; i++) {
        aggregate_field_t *field = vector_get(type->fields, i);
        if (streq(field->name, name)) {
            return i;
        }
    }

    return SIZE_MAX;
}

const type_t *get_field(const type_t *type, size_t idx) {
    UNUSED(idx);

    if (!is_aggregate(type)) {
        return type_poison("cannot take field from non aggregate");
    }

    if (vector_len(type->fields) >= idx) {
        return type_poison("out of bounds field access");
    }

    aggregate_field_t *field = vector_get(type->fields, idx);
    return field->type;
}

const char *get_poison_type_message(const type_t *type) {
    if (!is_poison(type)) {
        return "non-poison type";
    }

    return type->name;
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

    if (is_literal(lhs) && is_digit(rhs)) {
        return rhs;
    }

    if (is_digit(lhs) && is_literal(rhs)) {
        return lhs;
    }

    if (is_bool(lhs) && is_bool(rhs)) {
        return type_bool();
    }

    if (is_string(lhs) && is_string(rhs)) {
        return type_string();
    }

    char *err = format("cannot get common type of unrelated types (%s, %s)", type_format(lhs), type_format(rhs));
    return type_poison(err);
}
