#include "emit.h"

#include "ctu/util/str.h"

static const char *unary_op(unary_t unary) {
    switch (unary) {
    case UNARY_ABS: return "abs";
    case UNARY_NEG: return "neg";
    default: return NULL;
    }
}

static const char *binary_op(binary_t binary) {
    switch (binary) {
    case BINARY_ADD: return "add";
    case BINARY_SUB: return "sub";
    case BINARY_MUL: return "mul";
    case BINARY_DIV: return "div";
    case BINARY_REM: return "rem";

    case BINARY_EQ: return "eq";
    case BINARY_NEQ: return "neq";
    case BINARY_LT: return "lt";
    case BINARY_LTE: return "lte";
    case BINARY_GT: return "gt";
    case BINARY_GTE: return "gte";

    default: return NULL;
    }
}

static char *emit_imm(value_t *imm) {
    type_t *type = imm->type;
    if (type->type == TY_DIGIT) {
        return format("%s", mpz_get_str(NULL, 10, imm->digit));
    } else {
        return NULL;
    }
}

static char *emit_addr(block_t *addr) {
    return format("@%s", addr->name);
}

static char *emit_operand(operand_t op) {
    switch (op.kind) {
    case VREG: return format("%%%zu", op.vreg);
    case LABEL: return format(".%zu", op.label);
    case IMM: return emit_imm(op.imm);
    case ADDRESS: return emit_addr(op.block);
    default: return NULL;
    }
}

static char *emit_unary(size_t idx, step_t step) {
    const char *op = unary_op(step.unary);
    char *operand = emit_operand(step.operand);
    
    return format("%%%zu = unary %s %s", idx, op, operand);
}

static char *emit_binary(size_t idx, step_t step) {
    const char *op = binary_op(step.binary);
    char *lhs = emit_operand(step.lhs);
    char *rhs = emit_operand(step.rhs);

    return format("%%%zu = binary %s %s %s", idx, op, lhs, rhs);
}

static char *emit_return(step_t step) {
    char *operand = emit_operand(step.operand);
    return format("return %s", operand);
}

static char *emit_load(size_t idx, step_t step) {
    char *addr = emit_operand(step.src);
    char *offset = emit_operand(step.offset);
    return format("%%%zu = load %s +%s", idx, addr, offset);
}

static char *emit_step(block_t *flow, size_t idx) {
    step_t step = flow->steps[idx];
    switch (step.opcode) {
    case OP_EMPTY: return NULL;
    case OP_BINARY: return emit_binary(idx, step);
    case OP_UNARY: return emit_unary(idx, step);
    case OP_RETURN: return emit_return(step);
    case OP_LOAD: return emit_load(idx, step);
    default: return NULL;
    }
}

static void var_print(FILE *out, module_t *mod, size_t idx) {
    block_t *flow = vector_get(mod->vars, idx);
    const char *name = flow->name;
    
    size_t len = flow->len;
    fprintf(out, "value %s: %s {\n", name, type_format(flow->result));

    for (size_t i = 0; i < len; i++) {
        char *step = emit_step(flow, i);
        if (step != NULL) {
            fprintf(out, "  %s\n", step);
        }
    }

    fprintf(out, "}\n");
}

static void func_print(FILE *out, module_t *mod, size_t idx) {
    block_t *flow = vector_get(mod->funcs, idx);
    const char *name = flow->name;
    
    size_t len = flow->len;
    fprintf(out, "define %s: %s {\n", name, type_format(flow->result));

    for (size_t i = 0; i < len; i++) {
        char *step = emit_step(flow, i);
        if (step != NULL) {
            fprintf(out, "  %s\n", step);
        }
    }

    fprintf(out, "}\n");
}

void module_print(FILE *out, module_t *mod) {
    size_t nvars = vector_len(mod->vars);
    size_t nfuncs = vector_len(mod->funcs);

    fprintf(out, "module = %s\n", mod->name);
    for (size_t i = 0; i < nvars; i++) {
        var_print(out, mod, i);
    }

    for (size_t i = 0; i < nfuncs; i++) {
        func_print(out, mod, i);
    }
}
