#include "type.h"

#include "cthulhu/gen/value.h"
#include "cthulhu/gen/module.h"
#include "cthulhu/util/str.h"

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
    if (is_const(ptr)) {
        type = format("const %s", type);
    }
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

static const char *flatten_array(reports_t *reports, const type_t *type, const char *name) {
    vector_t *sizes = vector_new(4);
    const type_t *base = type;
    while (is_array(base)) {
        size_t size = static_array_length(base);
        vector_push(&sizes, (void*)size);
        base = index_type(base);
    }

    const char *inner = type_to_string(reports, base, name);
    size_t len = vector_len(sizes);
    for (size_t i = 0; i < len; i++) {
        const size_t size = (const size_t)vector_get(sizes, i);
        inner = format("%s[%zu]", inner, size);
    }

    return inner;
}

static const char *array_to_string(reports_t *reports, const type_t *type, const char *name) {
    return flatten_array(reports, type, name);
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

    if (is_array(type)) {
        return array_to_string(reports, type, name);
    }

    ctu_assert(reports, "unknown type `%s`", type_format(type));

    return "...";
}

static char *format_array(reports_t *reports, vector_t *elements) {
    size_t len = vector_len(elements);
    vector_t *result = vector_of(len);

    for (size_t i = 0; i < len; i++) {
        value_t *val = vector_get(elements, i);
        const char *fmt = value_to_string(reports, val);
        vector_set(result, i, (char*)fmt);
    }

    return format("{%s}", strjoin(", ", result));
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

    if (is_array(type)) {
        return format_array(reports, value->elements);
    }

    ctu_assert(reports, "unknown value type `%s`", type_format(type));

    return "???";
}
