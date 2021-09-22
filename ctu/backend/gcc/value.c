#include "value.h"

#include <string.h>

gcc_jit_rvalue *rvalue_from_value(gcc_jit_context *context, const value_t *value) {
    const type_t *vtype = value->type;
    gcc_jit_type *type = select_gcc_type(context, vtype);

    if (is_digit(vtype)) {
        return gcc_jit_context_new_rvalue_from_long(context, type, mpz_get_si(value->digit));
    }

    if (is_bool(vtype)) {
        if (value->boolean) {
            return gcc_jit_context_one(context, type);
        } else {
            return gcc_jit_context_zero(context, type);
        }
    }

    return NULL;
}

static size_t blob_digit_size(digit_t digit) {
    switch (digit.kind) {
    case TY_CHAR: return sizeof(char);
    case TY_SHORT: return sizeof(short);
    case TY_INT: return sizeof(int);
    case TY_LONG: return sizeof(long);
    case TY_SIZE: return sizeof(size_t);
    default: return 0;
    }
}

static size_t blob_type_size(const type_t *type) {
    if (is_bool(type)) {
        return sizeof(bool);
    }

    if (is_string(type)) {
        return sizeof(char*);
    }

    if (is_pointer(type)) {
        return sizeof(void*);
    }

    if (is_closure(type)) {
        return sizeof(void(*)(void));
    }

    if (is_digit(type)) {
        return blob_digit_size(type->digit);
    }

    return 0;
}

void *blob_from_value(const value_t *value, size_t *size) {
    const type_t *type = value->type;
    size_t bytes = blob_type_size(type);
    vector_t *blob = ctu_malloc(bytes);
    
    if (is_bool(type)) {
        *(bool*)blob = value->boolean;
    } else if (is_digit(type)) {
        unsigned long val = mpz_get_ui(value->digit);
        memcpy(blob, &val, bytes);
    } else {
        *size = 0;
        return NULL;
    }

    *size = bytes;
    return blob;
}
