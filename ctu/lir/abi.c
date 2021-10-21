#include "abi.h"

#include <string.h>

static char *mangle_closure(const type_t *type);

static const char *mangle_digit(digit_t digit) {
    switch (digit.kind) {
    case TY_CHAR: return digit.sign == SIGNED ? "c" : "a";
    case TY_SHORT: return digit.sign == SIGNED ? "s" : "t";
    case TY_INT: return digit.sign == SIGNED ? "i" : "j";
    case TY_LONG: return digit.sign == SIGNED ? "l" : "m";
    case TY_INTMAX: return digit.sign == SIGNED ? "x" : "y";
    default: return "";
    }
}

/**
 * 5.1.5.2 builtin types
 * https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-builtin
 */
static const char *builtin_mangle(const type_t *type) {
    const char *result = "";
    if (is_digit(type)) {
        result = mangle_digit(type->digit);
    } else if (is_void(type)) {
        result = "v";
    } else if (is_bool(type)) {
        result = "b";
    } else if (is_pointer(type)) {
        result = format("P%s", builtin_mangle(type->ptr));
    } else if (is_closure(type)) {
        result = mangle_closure(type);
    }

    return result;
}

static const char *mangle_params(vector_t *params) {
    size_t len = vector_len(params);
    if (len == 0) {
        return "v";
    }

    vector_t *it = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        const type_t *type = vector_get(params, i);
        vector_set(it, i, (char*)builtin_mangle(type));
    }
    
    return strjoin("", it);
}

static char *mangle_closure(const type_t *type) {
    const char *params = mangle_params(closure_params(type));
    return format("DoF%sE", params);
}

static char *mangle_part(const char *str) {
    size_t len = strlen(str);
    return format("%zu%s", len, str);
}

static char *mangle_path(vector_t *path) {
    size_t len = vector_len(path);
    vector_t *it = vector_of(len);

    for (size_t i = 0; i < len; i++) {
        const char *str = vector_get(path, i);
        vector_set(it, i, mangle_part(str));
    }
    
    char *result = strjoin("", it);
    if (len > 1) {
        return format("N%s", result);
    }

    return result;
}

char *mangle_name(vector_t *parts, const type_t *type) {
    vector_t *params = closure_params(type);
    const char *args = mangle_params(params);
    const char *path = mangle_path(parts);

    return format("_Z%sE%s", path, args);
}
