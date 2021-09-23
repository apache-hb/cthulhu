#include "type.h"

#include "ctu/util/str.h"

static const char *digit_name(int_t kind, bool sign) {
    switch (kind) {
    case TY_CHAR: return sign ? "signed char" : "unsigned char";
    case TY_SHORT: return sign ? "signed short" : "unsigned short";
    case TY_INT: return sign ? "signed int" : "unsigned int";
    case TY_LONG: return sign ? "signed long" : "unsigned long";
    case TY_SIZE: return sign ? "ssize_t" : "size_t";
    case TY_INTPTR: return sign ? "intptr_t" : "uintptr_t";
    case TY_INTMAX: return sign ? "intmax_t" : "uintmax_t";
    default: return "???";
    }
}

static const char *digit_to_string(digit_t digit, const char *name) {
    const char *type = digit_name(digit.kind, digit.sign == SIGNED);

    if (name != NULL) {
        return format("%s %s", type, name);
    }

    return type;
}

static const char *bool_to_string(const char *name) {
    if (name != NULL) {
        return format("bool %s", name);
    } else {
        return "bool";
    }
}

static const char *string_to_string(const char *name) {
    if (name != NULL) {
        return format("const char *%s", name);
    } else {
        return "const char *";
    }
}

const char *type_to_string(const type_t *type, const char *name) {
    if (is_digit(type)) {
        return digit_to_string(type->digit, name);
    }

    if (is_bool(type)) {
        return bool_to_string(name);
    }

    if (is_string(type)) {
        return string_to_string(name);
    }

    return NULL;
}

const char *value_to_string(const value_t *value) {
    const type_t *type = value->type;
    const char *cast = type_to_string(type, NULL);

    if (is_digit(type)) {
        char *digit = mpz_get_str(NULL, 10, value->digit);
        return format("(%s)%s", cast, digit);
    }

    if (is_bool(type)) {
        return value->boolean ? "true" : "false";
    }

    return NULL;
}
