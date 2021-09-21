#include "type.h"

static gcc_jit_type *select_gcc_int_type(gcc_jit_context *ctx, digit_t digit) {
    switch (digit.kind) {
    case TY_CHAR:
        return gcc_jit_context_get_type(
            ctx, 
            digit.sign == SIGNED ? GCC_JIT_TYPE_SIGNED_CHAR : GCC_JIT_TYPE_UNSIGNED_CHAR
        );
    case TY_SHORT:
        return gcc_jit_context_get_type(
            ctx,
            digit.sign == SIGNED ? GCC_JIT_TYPE_SHORT : GCC_JIT_TYPE_UNSIGNED_SHORT
        );
    case TY_INT:
        return gcc_jit_context_get_type(
            ctx,
            digit.sign == SIGNED ? GCC_JIT_TYPE_INT : GCC_JIT_TYPE_UNSIGNED_INT
        );
    case TY_LONG:
        return gcc_jit_context_get_type(
            ctx,
            digit.sign == SIGNED ? GCC_JIT_TYPE_LONG : GCC_JIT_TYPE_UNSIGNED_LONG
        );

    default:
        return NULL;
    }
}

static gcc_jit_type *select_gcc_string_type(gcc_jit_context *ctx) {
    return gcc_jit_context_get_type(
        ctx,
        GCC_JIT_TYPE_CONST_CHAR_PTR
    );
}

static gcc_jit_type *select_gcc_bool_type(gcc_jit_context *ctx) {
    return gcc_jit_context_get_type(
        ctx,
        GCC_JIT_TYPE_BOOL
    );
}

static gcc_jit_type *select_gcc_void_type(gcc_jit_context *ctx) {
    return gcc_jit_context_get_type(
        ctx,
        GCC_JIT_TYPE_VOID
    );
}

gcc_jit_type *select_gcc_type(gcc_jit_context *ctx, const type_t *type) {
    if (is_digit(type)) {
        return select_gcc_int_type(ctx, type->digit);
    }

    if (is_string(type)) {
        return select_gcc_string_type(ctx);
    }

    if (is_bool(type)) {
        return select_gcc_bool_type(ctx);
    }

    if (is_void(type)) {
        return select_gcc_void_type(ctx);
    }

    return NULL;
}
