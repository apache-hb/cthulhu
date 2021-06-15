#include "gcc.h"

#include "cthulhu/util/report.h"

#include <libgccjit.h>

typedef struct {
    gcc_jit_context *gcc;
    gcc_jit_type *lltype;
} gcc_t;

typedef struct {
    gcc_t *parent;
    gcc_jit_function *func;
    flow_t *flow;

    gcc_jit_rvalue **locals;

    size_t index;
    gcc_jit_block **blocks;
} path_t;

bool gcc_enabled(void) {
    return true;
}

static gcc_jit_block *cursor(path_t *path) {
    return path->blocks[path->index];
}

static gcc_jit_rvalue *operand_value(path_t *ctx, operand_t op) {
    switch (op.kind) {
    case IMM: 
        return 
            gcc_jit_context_new_rvalue_from_long(
                ctx->parent->gcc, ctx->parent->lltype, op.imm
            );
    case VREG:
        return ctx->locals[op.vreg];
    default:
        reportf("operand_value(op.kind = %d)", op.kind);
        return NULL;
    }
}

static void gcc_compile_value(path_t *path, op_t *op, size_t idx) {
    operand_t expr = op->expr;
    ASSERT(expr.kind != NAME);

    gcc_jit_rvalue *value = operand_value(path, expr);

    gcc_jit_lvalue *self = gcc_jit_function_new_local(
        path->func, NULL, path->parent->lltype, 
        format("local-%zu", idx)
    );

    gcc_jit_rvalue *obj = gcc_jit_lvalue_as_rvalue(self);

    gcc_jit_block_add_assignment(cursor(path), NULL, self, value);

    path->locals[idx] = obj;
}

static void gcc_select_block(path_t *path, size_t idx) {
    gcc_jit_block *last = cursor(path);
    path->index = idx;
    gcc_jit_block *now = cursor(path);

    gcc_jit_block_end_with_jump(last, NULL, now);
}

static void gcc_compile_ret(path_t *path, op_t *op) {
    gcc_jit_block_end_with_return(
        cursor(path), NULL, operand_value(path, op->expr)
    );
}

static op_t *flow_at(flow_t *flow, size_t idx) {
    return flow->ops + idx;
}

static void gcc_compile_op(path_t *path, size_t idx) {
    op_t *op = flow_at(path->flow, idx);

    switch (op->kind) {
    case OP_VALUE:
        gcc_compile_value(path, op, idx);
        break;

    case OP_RET:
        gcc_compile_ret(path, op);
        break;

    case OP_BLOCK:
        gcc_select_block(path, idx);
        break;

    default:
        reportf("gcc_compile_op([%zu] = %d)", idx, op->kind);
        break;
    }
}

static void gcc_insert_block(path_t *path, size_t idx) {
    path->blocks[idx + 1] = gcc_jit_function_new_block(
        path->func, format("block-%zu", idx)
    );
}

static void gcc_compile_flow(gcc_t *ctx, flow_t *flow) {
    gcc_jit_context *gcc = ctx->gcc;

    gcc_jit_function *func = gcc_jit_context_new_function(gcc, NULL,
        GCC_JIT_FUNCTION_EXPORTED,
        ctx->lltype, flow->name,
        0, NULL, 0
    );

    gcc_jit_rvalue **locals = malloc(sizeof(gcc_jit_rvalue*) * flow->len);
    for (size_t i = 0; i < flow->len; i++)
        locals[i] = NULL;

    size_t block_len = flow->len + 1;

    gcc_jit_block **blocks = malloc(sizeof(gcc_jit_block*) * block_len);
    for (size_t i = 0; i < block_len; i++)
        blocks[i] = NULL;

    blocks[0] = gcc_jit_function_new_block(func, "entry");

    path_t path = { ctx, func, flow, locals, 0, blocks };

    for (size_t i = 0; i < flow->len; i++) {
        if (flow_at(flow, i)->kind == OP_BLOCK)
            gcc_insert_block(&path, i);
    }

    for (size_t i = 0; i < flow->len; i++) {
        gcc_compile_op(&path, i);
    }
}

gcc_context *gcc_compile(unit_t *unit, bool debug) {
    gcc_jit_context *gcc = gcc_jit_context_acquire();

    if (debug) {
        gcc_jit_context_set_bool_option(gcc, GCC_JIT_BOOL_OPTION_DUMP_GENERATED_CODE, 0);
        gcc_jit_context_set_bool_option(gcc, GCC_JIT_BOOL_OPTION_DUMP_INITIAL_GIMPLE, 0);
    }

    gcc_t *ctx = malloc(sizeof(gcc_t));
    ctx->gcc = gcc;

    ctx->lltype = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_LONG_LONG);

    for (size_t i = 0; i < unit->len; i++) {
        gcc_compile_flow(ctx, unit->flows + i);
    }

    return ctx;
}

void gcc_output(gcc_context *ctx, const char *file) {
    gcc_t *self = (gcc_t*)ctx;
    gcc_jit_context_compile_to_file(self->gcc, GCC_JIT_OUTPUT_KIND_EXECUTABLE, file);

    const char *err = gcc_jit_context_get_first_error(self->gcc);

    if (err) {
        reportf(err);
    }
}
