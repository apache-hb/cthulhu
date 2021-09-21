#include "gcc.h"

#include <libgccjit.h>

#include "ctu/util/str.h"

#if 0
typedef struct {
    reports_t *reports;
    module_t *module;
    gcc_jit_context *context;
} context_t;

typedef struct {
    block_t *block;

    gcc_jit_block *current_path;
    gcc_jit_function *function;
    gcc_jit_param **params;
    gcc_jit_lvalue **locals;
    void **steps;
} gcc_block_t;

static gcc_jit_location *location_from_node(context_t *ctx, const node_t *node) {
    if (node == NULL) {
        fprintf(stderr, "node is NULL\n");
        return NULL;
    }

    gcc_jit_context *context = ctx->context;
    const char *path = node->scan->path;
    int line = node->where.first_line;
    int column = node->where.first_column;

    return gcc_jit_context_new_location(
        /* ctxt = */ context, 
        /* path = */ path, 
        /* line */ line, 
        /* column = */ column
    );
}

static gcc_jit_type *gcc_bool_type(context_t *ctx) {
    return gcc_jit_context_get_type(ctx->context, GCC_JIT_TYPE_BOOL);
}

static gcc_jit_type *gcc_type_of(context_t *ctx, const type_t *type) {
    if (is_digit(type)) {
        digit_t digit = type->digit;
        switch (digit.kind) {
        case TY_CHAR: 
            return gcc_jit_context_get_type(
                ctx->context, 
                digit.sign ? GCC_JIT_TYPE_SIGNED_CHAR : GCC_JIT_TYPE_UNSIGNED_CHAR
            );
        case TY_SHORT:
            return gcc_jit_context_get_type(
                ctx->context,
                digit.sign ? GCC_JIT_TYPE_SHORT : GCC_JIT_TYPE_UNSIGNED_SHORT
            );
        case TY_INT:
            return gcc_jit_context_get_type(
                ctx->context,
                digit.sign ? GCC_JIT_TYPE_INT : GCC_JIT_TYPE_UNSIGNED_INT
            );
        case TY_LONG:
            return gcc_jit_context_get_type(
                ctx->context,
                digit.sign ? GCC_JIT_TYPE_LONG : GCC_JIT_TYPE_UNSIGNED_LONG
            );
        default: return NULL;
        }
    }

    if (is_void(type)) {
        return gcc_jit_context_get_type(ctx->context, GCC_JIT_TYPE_VOID);
    }

    if (is_bool(type)) {
        return gcc_bool_type(ctx);
    }

    if (is_string(type)) {
        return gcc_jit_type_get_const(
            gcc_jit_type_get_pointer(
                gcc_jit_context_get_type(ctx->context, GCC_JIT_TYPE_CHAR)
            )
        );
    }

    return NULL;
}

static vector_t *gcc_func_params(context_t *ctx, const type_t *type) {
    if (!is_closure(type)) {
        return NULL;
    }

    vector_t *args = type->args;
    size_t len = vector_len(args);

    if (is_variadic(type)) {
        len -= 1;
    }

    vector_t *result = vector_of(len);

    for (size_t i = 0; i < len; i++) {
        const type_t *arg = vector_get(args, i);
        gcc_jit_type *gcc_type = gcc_type_of(ctx, arg);
        gcc_jit_param *param = gcc_jit_context_new_param(
            ctx->context,
            NULL,
            gcc_type,
            format("param%zu", i)
        );
        vector_set(result, i, param);
    }

    return result;
}

static void add_locals_to_function(context_t *ctx, gcc_block_t *block) {
    gcc_jit_function *function = block->function;

    vector_t *locals = block->block->locals;
    size_t len = vector_len(locals);
    
    block->locals = NEW_ARRAY(gcc_jit_lvalue*, len);

    for (size_t i = 0; i < len; i++) {
        const block_t *local = vector_get(locals, i);
        const char *name = local->name;
        const type_t *type = local->type;

        gcc_jit_type *gcc_type = gcc_type_of(ctx, type);
        gcc_jit_lvalue *lvalue = gcc_jit_function_new_local(
            /* func = */ function,
            /* loc = */ location_from_node(ctx, local->node),
            /* type = */ gcc_type,
            /* name = */ name
        );

        block->locals[i] = lvalue;
    }
}

static gcc_jit_function *gcc_jit_build_function(context_t *ctx,
                                               const node_t *node,
                                               const type_t *type,
                                               const char *name,
                                               enum gcc_jit_function_kind kind,
                                               gcc_jit_param ***params)
{
    gcc_jit_location *loc = location_from_node(ctx, node);
    gcc_jit_type *result = gcc_type_of(ctx, type->result);
    vector_t *args = gcc_func_params(ctx, type);

    if (params != NULL) {
        *params = (gcc_jit_param**)vector_data(args);
    }

    return gcc_jit_context_new_function(
        /* ctxt = */ ctx->context, 
        /* loc = */ loc,
        /* kind = */ kind,
        /* return_type = */ result,
        /* name = */ name,
        /* num_params = */ vector_len(args),
        /* params = */ (gcc_jit_param**)vector_data(args),
        /* is_variadic = */ is_variadic(type)
    );
}

static gcc_block_t *gcc_function_from_block(context_t *ctx, block_t *block) {
    size_t len = block->len;

    gcc_block_t *gcc_block = NEW(gcc_block_t);
    gcc_block->block = block;
    gcc_block->steps = NEW_ARRAY(void*, len);
    gcc_block->function = gcc_jit_build_function(
        ctx, block->node,
        block->type, block->name,
        GCC_JIT_FUNCTION_EXPORTED,
        &gcc_block->params
    );

    gcc_block->current_path = gcc_jit_function_new_block(gcc_block->function, "entry");

    for (size_t i = 0; i < len; i++) {
        step_t step = block->steps[i];
        if (step.opcode == OP_BLOCK) {
            gcc_block->steps[i] = gcc_jit_function_new_block(
                gcc_block->function,
                format("block%zu", i)
            );
        }
    }

    add_locals_to_function(ctx, gcc_block);

    block->data = gcc_block;

    return gcc_block;
}

static context_t *gcc_context_for_module(reports_t *reports, module_t *module) {
    gcc_jit_context *gcc = gcc_jit_context_acquire();

    if (gcc == NULL) {
        assert2(reports, "failed to create gccjit context");
        return NULL;
    }

    context_t *context = NEW(context_t); 
    
    context->reports = reports;
    context->module = module;
    context->context = gcc;

    return context;
}

static gcc_jit_lvalue *gcc_global_from_block(context_t *ctx, block_t *block) {
    gcc_jit_lvalue *global = gcc_jit_context_new_global(
        /* ctxt = */ ctx->context,
        /* loc = */ location_from_node(ctx, block->node),
        /* kind = */ GCC_JIT_GLOBAL_EXPORTED,
        /* type = */ gcc_type_of(ctx, block->type),
        /* name = */ block->name
    );

    block->data = global;

    return global;
}

static void begin_vars(context_t *ctx, module_t *mod) {
    vector_t *vars = mod->vars;
    size_t nvars = vector_len(vars);

    vector_t *result = vector_of(nvars);
    for (size_t i = 0; i < nvars; i++) {
        block_t *var = vector_get(vars, i);
        gcc_jit_lvalue *gcc_var = gcc_global_from_block(ctx, var);

        vector_set(result, i, gcc_var);
    }
}

static vector_t *begin_funcs(context_t *ctx, module_t *mod) {
    vector_t *funcs = mod->funcs;
    size_t nfuncs = vector_len(funcs);

    vector_t *result = vector_of(nfuncs);
    for (size_t i = 0; i < nfuncs; i++) {
        block_t *block = vector_get(funcs, i);
        gcc_block_t *gcc_block = gcc_function_from_block(ctx, block);

        vector_set(result, i, gcc_block);
    }

    return result;
}

static gcc_jit_rvalue *gcc_build_imm(context_t *ctx, value_t *value) {
    const type_t *type = value->type;
    if (is_bool(type)) {
        if (value->boolean) {
            return gcc_jit_context_one(
                ctx->context,
                gcc_bool_type(ctx)
            );
        } else {
            return gcc_jit_context_zero(
                ctx->context,
                gcc_bool_type(ctx)
            );
        }
    }

    if (is_digit(type)) {
        gcc_jit_type *gcc_type = gcc_type_of(ctx, type);
        return gcc_jit_context_new_rvalue_from_long(
            ctx->context,
            gcc_type,
            mpz_get_si(value->digit)
        );
    }

    return NULL;
}

static gcc_jit_rvalue *gcc_build_vreg(gcc_block_t *block, vreg_t vreg) {
    void *lvalue = block->steps[vreg];
    if (lvalue == NULL) {
        printf("gcc-build-vreg[%zu] = NULL\n", vreg);
    }
    return gcc_jit_lvalue_as_rvalue(lvalue);
}

static gcc_jit_rvalue *gcc_build_arg(gcc_block_t *block, vreg_t arg) {
    return gcc_jit_param_as_rvalue(block->params[arg]);
}

static gcc_jit_rvalue *gcc_build_rvalue(context_t *ctx, gcc_block_t *block, operand_t operand) {
    switch (operand.kind) {
    case IMM: 
        return gcc_build_imm(ctx, operand.imm);
    case VREG: 
        return gcc_build_vreg(block, operand.vreg);
    case ARG:
        return gcc_build_arg(block, operand.arg);
    case ADDRESS: 
        return operand.block->data;
        
    default: 
        printf("gcc-build-rvalue: %d\n", operand.kind);
        return NULL;
    }
}

static gcc_jit_lvalue *gcc_get_lvalue(operand_t operand) {
    switch (operand.kind) {
    case ADDRESS: return operand.block->data;

    default: 
        printf("gcc-get-lvalue: %d\n", operand.kind);
        return NULL;
    }
}

static gcc_jit_block *gcc_find_block(gcc_block_t *block, operand_t op) {
    if (op.kind == LABEL) {
        return block->steps[op.label];
    }

    return NULL;
}

static enum gcc_jit_binary_op gcc_find_binary(binary_t binary) {
    switch (binary) {
    case BINARY_ADD: return GCC_JIT_BINARY_OP_PLUS;
    case BINARY_SUB: return GCC_JIT_BINARY_OP_MINUS;
    case BINARY_MUL: return GCC_JIT_BINARY_OP_MULT;
    case BINARY_DIV: return GCC_JIT_BINARY_OP_DIVIDE;
    case BINARY_REM: return GCC_JIT_BINARY_OP_MODULO;
    case BINARY_BITAND: return GCC_JIT_BINARY_OP_BITWISE_AND;
    case BINARY_BITOR: return GCC_JIT_BINARY_OP_BITWISE_OR;
    case BINARY_XOR: return GCC_JIT_BINARY_OP_BITWISE_XOR;
    case BINARY_SHL: return GCC_JIT_BINARY_OP_LSHIFT;
    case BINARY_SHR: return GCC_JIT_BINARY_OP_RSHIFT;
    case BINARY_AND: return GCC_JIT_BINARY_OP_LOGICAL_AND;
    case BINARY_OR: return GCC_JIT_BINARY_OP_LOGICAL_OR;

    default: 
        return ~0;
    }
}

static enum gcc_jit_comparison gcc_find_compare(binary_t binary) {
    switch (binary) {
    case BINARY_EQ: return GCC_JIT_COMPARISON_EQ;
    case BINARY_NEQ: return GCC_JIT_COMPARISON_NE;
    case BINARY_LT: return GCC_JIT_COMPARISON_LT;
    case BINARY_LTE: return GCC_JIT_COMPARISON_LE;
    case BINARY_GT: return GCC_JIT_COMPARISON_GT;
    case BINARY_GTE: return GCC_JIT_COMPARISON_GE;

    default:
        return ~0;
    }
}

static gcc_jit_rvalue *gcc_build_binary(context_t *ctx, gcc_block_t *block, step_t step) {
    binary_t binary = step.binary;

    gcc_jit_location *location = location_from_node(ctx, step.node);

    gcc_jit_rvalue *lhs = gcc_build_rvalue(ctx, block, step.lhs);
    gcc_jit_rvalue *rhs = gcc_build_rvalue(ctx, block, step.rhs);

    gcc_jit_type *type = gcc_type_of(ctx, step.type);

    enum gcc_jit_binary_op op = gcc_find_binary(binary);
    if ((int)op != ~0) {
        return gcc_jit_context_new_binary_op(
            /* context = */ ctx->context,
            /* loc = */ location,
            /* op = */ op,
            /* result_type = */ type,
            /* a = */ lhs,
            /* b = */ rhs
        );
    } else {
        return gcc_jit_context_new_comparison(
            /* context = */ ctx->context,
            /* loc = */ location,
            /* op = */ gcc_find_compare(binary),
            /* a = */ lhs,
            /* b = */ rhs
        );
    }
}

static gcc_jit_lvalue *gcc_build_load(context_t *ctx, gcc_block_t *block, size_t idx, step_t step) {
    gcc_jit_location *location = location_from_node(ctx, step.node);

    gcc_jit_lvalue *dst = gcc_jit_function_new_local(
        block->function, location,
        gcc_type_of(ctx, step.type),
        format("temp%zu", idx)
    );
    gcc_jit_block_add_assignment(
        block->current_path, location,
        dst, gcc_build_rvalue(ctx, block, step.src)
    );

    return dst;
}

static void gcc_build_store(context_t *ctx, gcc_block_t *block, step_t step) {
    gcc_jit_block_add_assignment(
        block->current_path,
        location_from_node(ctx, step.node),
        gcc_get_lvalue(step.dst),
        gcc_build_rvalue(ctx, block, step.src)
    );
}

static void compile_step(context_t *ctx, gcc_block_t *gcc_block, step_t step, size_t idx) {
    gcc_jit_block *path;

    switch (step.opcode) {
    case OP_EMPTY: break;
    case OP_BLOCK:
        path = gcc_block->steps[idx];
        if (gcc_block->current_path != NULL) {
            gcc_jit_block_end_with_jump(
                /* block = */ gcc_block->current_path,
                /* loc = */ location_from_node(ctx, step.node),
                /* target = */ path
            );
        }
        gcc_block->current_path = path;
        break;
    
    case OP_RETURN:
        if (step.operand.kind == EMPTY) {
            gcc_jit_block_end_with_void_return(
                /* block = */ gcc_block->current_path,
                /* loc = */ location_from_node(ctx, step.node)
            );
        } else {
            gcc_jit_block_end_with_return(
                /* block = */ gcc_block->current_path,
                /* loc = */ location_from_node(ctx, step.node),
                /* return_expr = */ gcc_build_rvalue(ctx, gcc_block, step.operand)
            );
        }
        gcc_block->current_path = NULL;
        break;

    case OP_JMP:
        gcc_jit_block_end_with_jump(
            /* block = */ gcc_block->current_path,
            /* loc = */ location_from_node(ctx, step.node),
            /* target = */ gcc_find_block(gcc_block, step.label)
        );
        gcc_block->current_path = NULL;
        break;

    case OP_BRANCH:
        gcc_jit_block_end_with_conditional(
            /* block = */ gcc_block->current_path,
            /* loc = */ location_from_node(ctx, step.node),
            /* boolval = */ gcc_build_rvalue(ctx, gcc_block, step.cond),
            /* on_true = */ gcc_find_block(gcc_block, step.label),
            /* on_false = */ gcc_find_block(gcc_block, step.other)
        );
        gcc_block->current_path = NULL;
        break;

    case OP_BINARY:
        gcc_block->steps[idx] = gcc_build_binary(ctx, gcc_block, step);
        break;

    case OP_LOAD:
        gcc_block->steps[idx] = gcc_build_load(ctx, gcc_block, idx, step);
        break;

    case OP_STORE:
        gcc_build_store(ctx, gcc_block, step);
        break;

    default:
        fprintf(stderr, "Unhandled opcode: %d at %zu\n", step.opcode, idx);
        break;
    }
}

static void compile_gcc_function(context_t *ctx, gcc_block_t *gcc_block) {
    block_t *block = gcc_block->block;
    size_t steps = block->len;
    for (size_t i = 0; i < steps; i++) {
        step_t step = block->steps[i];
        compile_step(ctx, gcc_block, step, i);
    }
}

static void compile_context(context_t *ctx, vector_t *funcs) {
    size_t nfuncs = vector_len(funcs);
    for (size_t i = 0; i < nfuncs; i++) {
        gcc_block_t *gcc_block = vector_get(funcs, i);
        compile_gcc_function(ctx, gcc_block);
    }
}

static void import_symbols(context_t *ctx, vector_t *symbols) {
    size_t nsymbols = vector_len(symbols);
    for (size_t i = 0; i < nsymbols; i++) {
        block_t *block = vector_get(symbols, i);
        gcc_jit_function *func = gcc_jit_build_function(
            ctx, block->node,
            block->type, block->name,
            GCC_JIT_FUNCTION_IMPORTED,
            NULL
        );

        block->data = func;
    }
}

bool gccjit_build(reports_t *reports, module_t *mod, const char *path) {
    context_t *context = gcc_context_for_module(reports, mod);
    if (context == NULL) {
        return false;
    }

    begin_vars(context, mod);
    vector_t *funcs = begin_funcs(context, mod);

    import_symbols(context, mod->imports);

    compile_context(context, funcs);

    gcc_jit_context_compile_to_file(
        /* ctxt = */ context->context,
        /* output_kind = */ GCC_JIT_OUTPUT_KIND_EXECUTABLE,
        /* output_path = */ path
    );

    return true;
}
#endif

typedef struct {
    block_t *block;
    gcc_jit_function *function;

    gcc_jit_block *path;
    gcc_jit_param **params;
    
    /* gcc_jit_lvalue|gcc_jit_rvalue|gcc_jit_block */
    void **steps;
} flow_t;

bool gccjit_build(reports_t *reports, module_t *mod, const char *path) {
    assert2(reports, "gccjit unimplemented %p %s", mod, path);
    return false;
}
