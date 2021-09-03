#include "type.h"

#include "ctu/util/util.h"
#include "ctu/util/str.h"
#include "ctu/util/report.h"

static const char *int_format(int_t kind) {
    switch (kind) {
    case TY_CHAR: return "char";
    case TY_SHORT: return "short";
    case TY_INT: return "int";
    case TY_LONG: return "long";
    default: return "";
    }
}

static char *digit_format(digit_t digit) {
    const char *ty = int_format(digit.kind);
    if (!digit.sign) {
        return format("u%s", ty);
    }
    return ctu_strdup(ty);
}

static char *closure_format(const type_t *type) {
    size_t len = vector_len(type->args);
    vector_t *args = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        const type_t *arg = vector_get(type->args, i);
        vector_set(args, i, type_format(arg));
    }

    char *all = strjoin(", ", args);

    char *result = type_format(type->result);

    return format("%s(%s)", result, all);
}

char *type_format(const type_t *type) {
    switch (type->type) {
    case TY_DIGIT: return digit_format(type->digit);
    case TY_VOID: return ctu_strdup("void");
    case TY_CLOSURE: return closure_format(type);
    default: return ctu_strdup("");
    }
}

static type_t *type_new(metatype_t meta) {
    type_t *type = ctu_malloc(sizeof(type_t));
    type->type = meta;
    return type;
}

type_t *type_digit(sign_t sign, int_t kind) {
    type_t *type = type_new(TY_DIGIT);
    digit_t digit = { sign, kind };
    type->digit = digit;
    return type;
}

type_t *type_closure(vector_t *args, type_t *result) {
    type_t *type = type_new(TY_CLOSURE);
    type->args = args;
    type->result = result;
    return type;
}

bool is_digit(const type_t *type) {
    return type->type == TY_DIGIT;
}

bool is_signed(const type_t *type) {
    if (!is_digit(type)) {
        assert("is-signed cannot operate on non-digit types");
        return false;
    }

    return type->digit.sign == SIGNED;
}

bool is_unsigned(const type_t *type) {
    if (!is_digit(type)) {
        assert("is-unsigned cannot operate on non-digit types");
        return false;
    }

    return type->digit.sign == UNSIGNED;
}
