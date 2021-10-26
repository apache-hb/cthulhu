#include "type.h"

#include "ctu/gen/value.h"
#include "ctu/gen/module.h"
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
        return format("_Bool %s", name);
    } else {
        return "_Bool";
    }
}

static const char *string_to_string(const char *name) {
    if (name != NULL) {
        return format("const char *%s", name);
    } else {
        return "const char *";
    }
}

static const char *void_to_string(const char *name) {
    if (name != NULL) {
        return format("void %s", name);
    } else {
        return "void";
    }
}

static const char *ptr_to_string(reports_t *reports, const type_t *ptr, const char *name) {
    const char *type = type_to_string(reports, ptr, NULL);
    if (name != NULL) {
        return format("%s* %s", type, name);
    } else {
        return format("%s*", type);
    }
}

static char *closure_to_string(reports_t *reports, const type_t *type, const char *name) {
    size_t len = num_params(type);
    vector_t *params = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        const char *param = type_to_string(reports, param_at(type, i), NULL);
        vector_set(params, i, (char*)param);
    }

    const char *result = type_to_string(reports, closure_result(type), NULL);
    char *args = strjoin(", ", params);

    if (name != NULL) {
        return format("%s (*%s)(%s)", result, name, args);
    } else {
        return format("%s (*)(%s)", result, args);
    }
}

const char *type_to_string(reports_t *reports, const type_t *type, const char *name) {
    if (is_digit(type)) {
        return digit_to_string(type->digit, name);
    }

    if (is_bool(type)) {
        return bool_to_string(name);
    }

    if (is_string(type)) {
        return string_to_string(name);
    }

    if (is_void(type)) {
        return void_to_string(name);
    }

    if (is_varargs(type)) {
        return "...";
    }

    if (is_literal(type)) {
        ctu_assert(reports, "cannot format untyped integer literal type");
        return "literal";
    }

    if (is_pointer(type)) {
        return ptr_to_string(reports, type->ptr, name);
    }

    if (is_closure(type)) {
        return closure_to_string(reports, type, name);
    }

    ctu_assert(reports, "unknown type `%s`", type_format(type));

    return "...";
}

const char *value_to_string(reports_t *reports, const value_t *value) {
    const type_t *type = value->type;
    const char *cast = type_to_string(reports, type, NULL);

    if (is_digit(type)) {
        char *digit = mpz_get_str(NULL, 10, value->digit);
        return format("(%s)%s", cast, digit);
    }

    if (is_bool(type)) {
        return value->boolean ? "1" : "0";
    }

    if (is_string(type)) {
        return format("\"%s\"", strnorm(value->string));
    }

    if (is_closure(type)) {
        return value->block->name;
    }

    if (is_literal(type)) {
        ctu_assert(reports, "cannot emit untyped integer literal value");
        return "literal";
    }

    /**
     * the only wait a pointer can be literal is if its the null pointer 
     * this might change in the future but for now this works
     */
    if (is_pointer(type)) {
        return format("(void*)0");
    }

    ctu_assert(reports, "unknown value type `%s`", type_format(type));

    return "???";
}
