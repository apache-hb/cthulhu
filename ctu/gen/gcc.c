#include <libgccjit.h>

#include "gcc.h"

#include "ctu/util/report.h"

typedef struct {
    module_t *mod;
    gcc_jit_context *ctx;

    gcc_jit_type *void_type;
    gcc_jit_type *bool_type;

    gcc_jit_type *char_type;
    gcc_jit_type *short_type;
    gcc_jit_type *int_type;
    gcc_jit_type *long_type;
    gcc_jit_type *long_long_type;

    gcc_jit_type *unsigned_char_type;
    gcc_jit_type *unsigned_short_type;
    gcc_jit_type *unsigned_int_type;
    gcc_jit_type *unsigned_long_type;
    gcc_jit_type *unsigned_long_long_type;

    gcc_jit_type *str_type;

    gcc_jit_rvalue **strings;

    gcc_jit_lvalue **globals;
} ctx_t;

static gcc_jit_type *select_int(ctx_t *ctx, integer_t ty, bool sign) {
    switch (ty) {
    case INTEGER_CHAR: return sign ? ctx->char_type : ctx->unsigned_char_type;
    case INTEGER_SHORT: return sign ? ctx->short_type : ctx->unsigned_short_type;
    case INTEGER_INT: return sign ? ctx->int_type : ctx->unsigned_int_type;
    case INTEGER_LONG: return sign ? ctx->long_type : ctx->unsigned_long_type;
    case INTEGER_SIZE: return sign ? ctx->long_type : ctx->unsigned_long_type;
    case INTEGER_INTPTR: return sign ? ctx->long_type : ctx->unsigned_long_type;
    case INTEGER_INTMAX: return sign ? ctx->long_long_type : ctx->unsigned_long_long_type;

    default:
        assert("select-int unknown type %d", ty);
        return ctx->int_type;
    }
}

static gcc_jit_type *select_type(ctx_t *ctx, type_t *ty) {
    switch (ty->kind) {
    case TYPE_INTEGER: 
        return select_int(ctx, ty->integer, ty->sign);
    case TYPE_BOOLEAN:
        return ctx->bool_type;
    case TYPE_VOID:
        return ctx->void_type;
    case TYPE_POINTER:
        return gcc_jit_type_get_pointer(select_type(ctx, ty->ptr));
    case TYPE_STRING: 
        return ctx->str_type;

    default:
        assert("unknown select-type");
        return ctx->bool_type;
    }
}

static enum gcc_jit_global_kind get_kind(bool exported, bool imported) {
    if (exported) {
        return GCC_JIT_GLOBAL_EXPORTED;
    } else if (imported) {
        return GCC_JIT_GLOBAL_IMPORTED;
    } else {
        return GCC_JIT_GLOBAL_INTERNAL;
    }
}

#define GET_KIND(it) (get_kind(it.exported, it.interop))

static ctx_t make_ctx(module_t *mod) {
    gcc_jit_context *gcc = gcc_jit_context_acquire();

    gcc_jit_context_set_str_option(gcc, GCC_JIT_STR_OPTION_PROGNAME, mod->name);
    gcc_jit_context_set_int_option(gcc, GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL, 2);

    ctx_t ctx;

    ctx.mod = mod;
    ctx.ctx = gcc;

    ctx.void_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_VOID);
    ctx.bool_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_BOOL);

    ctx.char_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_CHAR);
    ctx.short_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_SHORT);
    ctx.int_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_INT);
    ctx.long_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_LONG);
    ctx.long_long_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_LONG_LONG);

    ctx.unsigned_char_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_UNSIGNED_CHAR);
    ctx.unsigned_short_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_UNSIGNED_SHORT);
    ctx.unsigned_int_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_UNSIGNED_INT);
    ctx.unsigned_long_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_UNSIGNED_LONG);
    ctx.unsigned_long_long_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_UNSIGNED_LONG_LONG);

    ctx.str_type = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_CONST_CHAR_PTR);

    ctx.strings = ctu_malloc(sizeof(gcc_jit_rvalue*) * num_strings(mod));

    for (size_t i = 0; i < num_strings(mod); i++) {
        ctx.strings[i] = gcc_jit_context_new_string_literal(gcc, mod->strings[i]);
    }

    ctx.globals = ctu_malloc(sizeof(gcc_jit_lvalue*) * num_vars(mod));

    for (size_t i = 0; i < num_vars(mod); i++) {
        var_t var = mod->vars[i];
        ctx.globals[i] = gcc_jit_context_new_global(gcc, NULL, 
            GET_KIND(var), select_type(&ctx, var.type),
            var.name
        );
    }

    return ctx;
}

void gen_gcc(module_t *mod) {
    make_ctx(mod);

    for (size_t i = 0; i < num_vars(mod); i++) {

    }

    for (size_t i = 0; i < num_flows(mod); i++) {

    }
}
