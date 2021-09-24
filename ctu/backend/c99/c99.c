#include "c99.h"

#include "ctu/util/str.h"

#include "type.h"

typedef struct {
    reports_t *reports;
    module_t *mod;

    map_t *strings;
    stream_t *result;
} context_t;

static context_t *init_c99_context(reports_t *reports, module_t *mod) {
    context_t *ctx = NEW(context_t);
    ctx->reports = reports;
    ctx->mod = mod;
    ctx->strings = map_new(MAP_SMALL);
    ctx->result = stream_new(0x1000);
    return ctx;
}

static void free_c99_context(context_t *ctx) {
    stream_delete(ctx->result);
    DELETE(ctx);
}

static void forward_global(context_t *ctx, const block_t *block) {
    const type_t *type = block->type;
    const char *name = block->name;

    const char *decl = type_to_string(type, name);

    char *forward = format("%s;\n", decl);

    stream_write(ctx->result, forward);

    ctu_free(forward);
}

static void add_global(context_t *ctx, const block_t *block) {
    const type_t *type = block->type;
    const value_t *value = block->value;
    const char *name = block->name;

    const char *start = type_to_string(type, name);
    const char *init = value_to_string(value);

    char *fmt = format("%s = %s;\n", start, init);

    stream_write(ctx->result, fmt);

    ctu_free(fmt);
}

static void add_globals(context_t *ctx, vector_t *globals) {
    size_t len = vector_len(globals);
    
    stream_write(ctx->result, "\n// Global forwarding\n");
    
    for (size_t i = 0; i < len; i++) {
        const block_t *block = vector_get(globals, i);
        forward_global(ctx, block);
    }

    stream_write(ctx->result, "\n// Global initialization\n");

    for (size_t i = 0; i < len; i++) {
        const block_t *block = vector_get(globals, i);
        add_global(ctx, block);
    }
}

static char *format_function(const block_t *block) {
    const char *name = block->name;
    const type_t *type = block->type;
    const vector_t *args = type->args;

    size_t len = vector_len(args);
    vector_t *params = vector_of(vector_len(args));
    for (size_t i = 0; i < len; i++) {
        const type_t *arg = vector_get(args, i);
        const char *param = type_to_string(arg, NULL);
        vector_set(params, i, (char*)param);
    }
    
    const char *decl = type_to_string(type->result, name);

    return format("%s(%s)", decl, strjoin(", ", params));
}

static void forward_block(context_t *ctx, const block_t *block) {
    char *decl = format_function(block);
    stream_write(ctx->result, format("%s;\n", decl));
}

static char *format_vreg(vreg_t vreg) {
    return format("vreg%zu", vreg);
}

static char *string_name(size_t idx) {
    return format("strtab%zu", idx);
}

static char *format_addr(const block_t *block) {
    if (block->kind == BLOCK_STRING) {
        const block_t *real = block->data ?: block;
        return string_name(real->idx);
    }

    return format("&%s", block->name);
}

static const char *format_operand(operand_t op) {
    switch (op.kind) {
    case LABEL: return format("block%zu", op.label);
    case VREG: return format_vreg(op.vreg);
    case ADDRESS: return format_addr(op.block);
    case IMM: return value_to_string(op.imm);

    default: return format("unknown(%d)", op.kind);
    }
}

static char *format_branch(step_t step) {
    const char *cond = format_operand(step.cond);
    const char *label = format_operand(step.label);
    const char *other = format_operand(step.other);

    return format("  if (%s) { goto %s; } else { goto %s; }\n", cond, label, other);
}

static char *format_return(step_t step) {
    operand_t ret = step.operand;
    if (ret.kind == EMPTY) {
        return ctu_strdup("  return;\n");
    }

    return format("  return %s;\n", format_operand(ret));
}

static char *format_load(size_t idx, step_t step) {
    char *vreg = format_vreg(idx);
    const char *local = type_to_string(step.type, vreg);

    ctu_free(vreg);

    return format("  %s = *%s;\n", local, format_operand(step.src));
}

static char *format_store(step_t step) {
    const char *dst = format_operand(step.dst);
    const char *src = format_operand(step.src);

    return format("  *%s = %s;\n", dst, src);
}

static char *format_call(size_t idx, step_t step) {
    const char *init = "";
    if (!is_void(step.type)) {
        char *vreg = format_vreg(idx);
        init = format("%s = ", type_to_string(step.type, vreg));
    }

    const char *call = format_operand(step.func);
    vector_t *params = vector_of(step.len);
    for (size_t i = 0; i < step.len; i++) {
        operand_t arg = step.args[i];
        const char *param = format_operand(arg);
        vector_set(params, i, (char*)param);
    }

    return format("  %s(*%s)(%s);\n", init, call, strjoin(", ", params));
}

static const char *binary_op_to_string(binary_t op) {
    switch (op) {
    case BINARY_ADD: return "+";
    case BINARY_SUB: return "-";
    case BINARY_MUL: return "*";
    case BINARY_DIV: return "/";
    case BINARY_REM: return "%";

    case BINARY_EQ: return "==";
    case BINARY_NEQ: return "!=";
    case BINARY_LT: return "<";
    case BINARY_LTE: return "<=";
    case BINARY_GT: return ">";
    case BINARY_GTE: return ">=";

    default: return "???";
    }
}

static const char *unary_op_to_string(unary_t op, const char *operand) {
    switch (op) {
    case UNARY_ABS: return format("abs(%s)", operand);
    case UNARY_NEG: return format("-%s", operand);
    default: return "???";
    }
}

static char *format_binary(size_t idx, step_t step) {
    const char *vreg = format_vreg(idx);
    const char *temp = type_to_string(step.type, vreg);
    const char *lhs = format_operand(step.lhs);
    const char *rhs = format_operand(step.rhs);
    const char *op = binary_op_to_string(step.binary);

    return format("  %s = %s %s %s;\n", temp, lhs, op, rhs);
}

static char *format_unary(size_t idx, step_t step) {
    const char *vreg = format_vreg(idx);
    const char *temp = type_to_string(step.type, vreg);
    const char *operand = format_operand(step.operand);
    const char *op = unary_op_to_string(step.unary, operand);

    return format("  %s = %s;\n", temp, op);
}

static char *select_step(context_t *ctx, size_t idx, step_t step) {
switch (step.opcode) {
    case OP_EMPTY: return NULL;
    case OP_BLOCK: 
        return format("block%zu:\n", idx); 

    case OP_JMP:
        return format("  goto %s;\n", format_operand(step.label));

    case OP_BRANCH:
        return format_branch(step);

    case OP_RETURN:
        return format_return(step);

    case OP_LOAD:
        return format_load(idx, step);

    case OP_STORE:
        return format_store(step);

    case OP_CALL:
        return format_call(idx, step);

    case OP_UNARY:
        return format_unary(idx, step);

    case OP_BINARY:
        return format_binary(idx, step);

    default:
        assert2(ctx->reports, "unknown opcode: %d", step.opcode);
        return format("  // unimplemented operand %d at %zu\n", step.opcode, idx);
    }
}

static void write_step(context_t *ctx, size_t idx, step_t step) {
    char *code = select_step(ctx, idx, step);
    if (code == NULL) {
        return;
    }

    stream_write(ctx->result, code);

    ctu_free(code);
}

static void write_locals(context_t *ctx, const block_t *block) {
    const vector_t *locals = block->locals;
    size_t len = vector_len(locals);

    for (size_t i = 0; i < len; i++) {
        const block_t *local = vector_get(locals, i);
        const char *name = local->name;
        const type_t *type = local->type;
        const char *it = type_to_string(type, name);

        stream_write(ctx->result, format("  %s;\n", it));
    }
}

static void add_block(context_t *ctx, const block_t *block) {
    char *decl = format_function(block);
    stream_write(ctx->result, format("%s {\n", decl));

    write_locals(ctx, block);

    for (size_t i = 0; i < block->len; i++) {
        write_step(ctx, i, block->steps[i]);
    }

    stream_write(ctx->result, "}\n");
}

static void add_blocks(context_t *ctx, vector_t *blocks) {
    size_t len = vector_len(blocks);

    stream_write(ctx->result, "\n// Function forwarding\n");

    for (size_t i = 0; i < len; i++) {
        const block_t *block = vector_get(blocks, i);
        forward_block(ctx, block);
    }

    stream_write(ctx->result, "\n// Function definitions\n");

    for (size_t i = 0; i < len; i++) {
        const block_t *block = vector_get(blocks, i);
        add_block(ctx, block);
    }
}

static void add_strings(context_t *ctx, vector_t *strings) {
    size_t len = vector_len(strings);

    stream_write(ctx->result, "\n// String literals\n");

    for (size_t i = 0; i < len; i++) {
        block_t *block = vector_get(strings, i);
        
        block_t *other = map_get(ctx->strings, block->string);
        if (other != NULL && other != block) {
            block->data = other;
        } else {
            map_set(ctx->strings, block->string, block);
            block->data = NULL;
            char *name = string_name(block->idx);
            char *str = strnorm(block->string);
            char *fmt = format("const char *%s = \"%s\";\n", name, str);
            stream_write(ctx->result, fmt);
        }
    }
}

bool c99_build(reports_t *reports, module_t *mod, const char *path) {
    context_t *ctx = init_c99_context(reports, mod);
    
    char *header = format(
        "/**\n"
        " * Autogenerated by the Cthulhu Compiler Collection C99 backend\n"
        " * Generated from %s\n"
        " */\n",
        path
    );

    stream_write(ctx->result, header);

    ctu_free(header);

    add_strings(ctx, mod->strtab);

    add_globals(ctx, mod->vars);

    add_blocks(ctx, mod->funcs);

    file_t *file = ctu_open(path, "w");
    if (file == NULL) {
        assert2(ctx->reports, "failed to open file: %s", path);
        free_c99_context(ctx);
        return false;
    }

    fwrite(stream_data(ctx->result), 1, stream_len(ctx->result), file->file);

    ctu_close(file);
    free_c99_context(ctx);

    return true;
}
