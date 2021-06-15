#include "gcc.h"

#include "cthulhu/util/report.h"

#include <string.h>

#include <libgccjit.h>

typedef struct path_t path_t;

typedef struct {
    gcc_jit_context *gcc;
    gcc_jit_type *lltype;
    unit_t *unit;
    path_t **paths;
} gcc_t;

typedef struct path_t {
    gcc_t *parent;
    gcc_jit_function *func;
    flow_t *flow;

    gcc_jit_rvalue **locals;

    bool ended;
    gcc_jit_block **blocks;
    size_t block_track;
    gcc_jit_block *current;
} path_t;

bool gcc_enabled(void) {
    return true;
}

static gcc_jit_function *get_function(gcc_t *ctx, const char *name) {
    for (size_t i = 0; i < ctx->unit->len; i++) {
        path_t *path = ctx->paths[i];
        if (strcmp(path->flow->name, name) == 0) {
            return path->func;
        }
    }
    reportf("failed to find function `%s`", name);
    return NULL;
}

static gcc_jit_block *cursor(path_t *path) {
    return path->current;
}

static gcc_jit_block *get_block(path_t *path, size_t idx) {
    return path->blocks[idx];
}

static void mark_block_done(path_t *path, size_t idx) {
    if (path->block_track != SIZE_MAX)
        path->blocks[path->block_track] = NULL;

    path->block_track = idx;
}

static gcc_jit_rvalue *imm_rvalue(path_t *ctx, int64_t imm) {
    return gcc_jit_context_new_rvalue_from_long(
        ctx->parent->gcc, ctx->parent->lltype, imm
    );
}

static gcc_jit_rvalue *operand_value(path_t *ctx, operand_t op) {
    switch (op.kind) {
    case IMM: 
        return imm_rvalue(ctx, op.imm);
    case VREG:
        return ctx->locals[op.vreg];
    default:
        reportf("operand_value(op.kind = %d)", op.kind);
        return NULL;
    }
}

static void gcc_compile_value(path_t *path, op_t *op, size_t idx) {
    gcc_jit_rvalue *value = operand_value(path, op->expr);

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
    path->current = get_block(path, idx);
    gcc_jit_block *now = cursor(path);

    printf("select block %zu\n", idx);
    if (!path->ended) {
        gcc_jit_block_end_with_jump(last, NULL, now);
        mark_block_done(path, idx);
        path->ended = true;
    }
}

static void gcc_compile_ret(path_t *path, op_t *op) {
    gcc_jit_block_end_with_return(
        cursor(path), NULL, operand_value(path, op->expr)
    );
}

static op_t *flow_at(flow_t *flow, size_t idx) {
    return flow->ops + idx;
}

static void gcc_compile_unary(path_t *path, op_t *op, size_t idx, enum gcc_jit_unary_op unary) {
    path->locals[idx] = gcc_jit_context_new_unary_op(
        path->parent->gcc, NULL, unary,
        path->parent->lltype, operand_value(path, op->expr)
    );
}

static void gcc_compile_binary(path_t *path, op_t *op, size_t idx, enum gcc_jit_binary_op binop) {
    path->locals[idx] = gcc_jit_context_new_binary_op(
        path->parent->gcc, NULL, binop,
        path->parent->lltype, 
        operand_value(path, op->lhs),
        operand_value(path, op->rhs)
    );
}

static gcc_jit_block *new_block(path_t *path, const char *name) {
    return gcc_jit_function_new_block(path->func, name);
}

/**
 * turn 
 * %vreg = select cond true false
 * 
 * into
 * 
 * start:
 *   temp = uninit
 *   if (cond)
 *     goto true
 *   else
 *     goto false
 * true:
 *   temp = true
 *   goto end
 * false:
 *   temp = false
 *   goto end
 * end:
 *   
 */
static void gcc_compile_select(path_t *path, op_t *op, size_t idx) {
    gcc_jit_block *start = new_block(path, format("select-front-%zu", idx));

    gcc_jit_lvalue *temp = gcc_jit_function_new_local(path->func, NULL, 
        path->parent->lltype,
        format("select-val-%zu", idx)
    );

    gcc_jit_block *lhs = new_block(path, format("select-true-%zu", idx));
    gcc_jit_block *rhs = new_block(path, format("select-false-%zu", idx));

    gcc_jit_block *end = new_block(path, format("select-end-%zu", idx));

    gcc_jit_block_end_with_jump(cursor(path), NULL, start);

    gcc_jit_block_add_assignment(lhs, NULL, temp, operand_value(path, op->lhs));
    gcc_jit_block_add_assignment(rhs, NULL, temp, operand_value(path, op->rhs));

    gcc_jit_block_end_with_conditional(start, NULL, 
        operand_value(path, op->cond),
        lhs, rhs
    );

    gcc_jit_block_end_with_jump(lhs, NULL, end);
    gcc_jit_block_end_with_jump(rhs, NULL, end);

    path->current = end;
}

static void gcc_compile_branch(path_t *path, op_t *op) {
    printf("branch\n");
    gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(path->parent->gcc, NULL, 
        GCC_JIT_COMPARISON_NE, 
        operand_value(path, op->cond),
        imm_rvalue(path, 0)
    );
    gcc_jit_block_end_with_conditional(cursor(path), NULL,
        cmp,
        get_block(path, op->lhs.block),
        get_block(path, op->rhs.block)
    );
}

static void gcc_compile_jump(path_t *path, op_t *op) {
    printf("compile jump\n");
    gcc_jit_block_end_with_jump(cursor(path), NULL,
        get_block(path, op->label)
    );
}

static void gcc_compile_phi(path_t *path, op_t *op, size_t idx) {
    /* TODO: this is awful and broken */
    path->locals[idx] = operand_value(path, op->lhs);
}

static void gcc_compile_call(path_t *path, op_t *op, size_t idx) {
    gcc_jit_rvalue *call = gcc_jit_context_new_call(path->parent->gcc, NULL,
        get_function(path->parent, op->expr.name),
        0, NULL
    );
    gcc_jit_block_add_eval(cursor(path), NULL, call);
    path->locals[idx] = call;
}

static void gcc_compile_op(path_t *path, size_t idx) {
    op_t *op = flow_at(path->flow, idx);

    switch (op->kind) {
    case OP_VALUE:
        gcc_compile_value(path, op, idx);
        break;

    case OP_ABS:
        gcc_compile_unary(path, op, idx, GCC_JIT_UNARY_OP_ABS);
        break;
    case OP_NEG:
        gcc_compile_unary(path, op, idx, GCC_JIT_UNARY_OP_MINUS);
        break;

    case OP_ADD:
        gcc_compile_binary(path, op, idx, GCC_JIT_BINARY_OP_PLUS);
        break;
    case OP_SUB:
        gcc_compile_binary(path, op, idx, GCC_JIT_BINARY_OP_MINUS);
        break;
    case OP_DIV:
        gcc_compile_binary(path, op, idx, GCC_JIT_BINARY_OP_DIVIDE);
        break;
    case OP_MUL:
        gcc_compile_binary(path, op, idx, GCC_JIT_BINARY_OP_MULT);
        break;
    case OP_REM:
        gcc_compile_binary(path, op, idx, GCC_JIT_BINARY_OP_MODULO);
        break;

    case OP_CALL:
        gcc_compile_call(path, op, idx);
        break;

    case OP_SELECT:
        gcc_compile_select(path, op, idx);
        break;

    case OP_BRANCH:
        gcc_compile_branch(path, op);
        break;

    case OP_JUMP:
        gcc_compile_jump(path, op);
        break;

    case OP_PHI:
        gcc_compile_phi(path, op, idx);
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
    path->blocks[idx] = new_block(path, format("block-%zu", idx));
}

static void gcc_create_flow(gcc_t *ctx, flow_t *flow, size_t i) {
    gcc_jit_context *gcc = ctx->gcc;

    gcc_jit_function *func = gcc_jit_context_new_function(gcc, NULL,
        GCC_JIT_FUNCTION_EXPORTED,
        ctx->lltype, flow->name,
        0, NULL, 0
    );

    gcc_jit_rvalue **locals = malloc(sizeof(gcc_jit_rvalue*) * flow->len);
    for (size_t i = 0; i < flow->len; i++)
        locals[i] = NULL;

    size_t block_len = flow->len;

    gcc_jit_block **blocks = malloc(sizeof(gcc_jit_block*) * block_len);
    for (size_t i = 0; i < block_len; i++)
        blocks[i] = NULL;

    path_t *path = malloc(sizeof(path_t));
    path->parent = ctx;
    path->func = func;
    path->flow = flow;
    path->locals = locals;
    path->block_track = SIZE_MAX;
    path->blocks = blocks;
    path->ended = false;
    path->current = gcc_jit_function_new_block(func, "entry");

    for (size_t i = 0; i < flow->len; i++) {
        if (flow_at(flow, i)->kind == OP_BLOCK) {
            gcc_insert_block(path, i);
        }
    }

    ctx->paths[i] = path;
}

static void gcc_compile_flow(size_t len, path_t *path) {
    for (size_t i = 0; i < len; i++) {
        gcc_compile_op(path, i);
    }
}

gcc_context *gcc_compile(unit_t *unit, bool debug) {
    gcc_jit_context *gcc = gcc_jit_context_acquire();

    gcc_jit_context_set_str_option(gcc, GCC_JIT_STR_OPTION_PROGNAME, unit->name);

    if (debug) {
        gcc_jit_context_set_bool_option(gcc, GCC_JIT_BOOL_OPTION_DUMP_GENERATED_CODE, 1);
        gcc_jit_context_set_bool_option(gcc, GCC_JIT_BOOL_OPTION_DUMP_INITIAL_GIMPLE, 1);
    }

    gcc_t *ctx = malloc(sizeof(gcc_t));
    ctx->gcc = gcc;
    ctx->unit = unit;
    ctx->paths = malloc(sizeof(path_t*) * unit->len);
    ctx->lltype = gcc_jit_context_get_type(gcc, GCC_JIT_TYPE_LONG_LONG);

    for (size_t i = 0; i < unit->len; i++) {
        gcc_create_flow(ctx, unit->flows + i, i);
    }

    for (size_t i = 0; i < unit->len; i++) {
        gcc_compile_flow(unit->flows[i].len, ctx->paths[i]);
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
