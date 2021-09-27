#include "eval.h"

typedef struct {
    reports_t *reports;
    module_t *mod;
    block_t *block;
    size_t ip;

    value_t **values;
    value_t *result;
} exec_t;

static exec_t *exec_new(reports_t *reports, module_t *mod, block_t *block) {
    size_t size = block->len;

    value_t **values = ctu_malloc(sizeof(value_t*) * size);
    for (size_t i = 0; i < size; i++) {
        values[i] = value_poison("unintalized value");
    }

    exec_t *exec = ctu_malloc(sizeof(exec_t));
    exec->reports = reports;
    exec->mod = mod;
    exec->block = block;
    exec->ip = 0;
    exec->values = values;
    exec->result = value_poison("unintalized result");

    return exec;
}

static step_t get_step(exec_t *exec, size_t ip) {
    return exec->block->steps[ip];
}

static value_t *get_value(exec_t *exec, operand_t it) {
    switch (it.kind) {
    case IMM: return it.imm;
    case VREG: return exec->values[it.vreg];
    
    default:
        assert2(exec->reports, "invalid operand kind");
        return value_poison("invalid operand kind");
    }
}

static size_t get_label(exec_t *exec, operand_t it) {
    if (it.kind != LABEL) {
        assert2(exec->reports, "invalid operand kind");
        return SIZE_MAX;
    }

    return it.label;
}

static value_t *value_abs(value_t *v) {
    if (!is_digit(v->type)) {
        return value_poison("not a digit type");
    }

    mpz_t result;
    mpz_abs(result, v->digit);

    return value_digit(v->type, result);
}

static value_t *value_neg(value_t *v) {
    if (!is_digit(v->type)) {
        return value_poison("not a digit type");
    }

    mpz_t result;
    mpz_neg(result, v->digit);

    return value_digit(v->type, result);
}


static value_t *exec_unary(exec_t *exec, step_t step) {
    value_t *operand = get_value(exec, step.operand);

    switch (step.unary) {
    case UNARY_ABS: return value_abs(operand);
    case UNARY_NEG: return value_neg(operand);

    default:
        assert2(exec->reports, "invalid unary operator");
        return value_poison("invalid unary operator");
    }
}

static bool exec_step(exec_t *exec) {
    size_t here = exec->ip;
    
    if (here >= exec->block->len) {
        return false;
    }

    step_t step = get_step(exec, exec->ip++);

    switch (step.opcode) {
    case OP_RETURN:
        exec->result = get_value(exec, step.operand);
        return false;

    case OP_UNARY:
        exec->values[here] = exec_unary(exec, step);
        return true;

    case OP_JMP:
        exec->ip = get_label(exec, step.label);
        return exec->ip != SIZE_MAX;

    case OP_EMPTY: 
    case OP_BLOCK:
        return true;

    default:
        assert2(exec->reports, "%s invalid opcode: [%zu] = %d", exec->block->name, exec->ip, step.opcode);
        return false;
    }
}

value_t *eval_block(reports_t *reports, module_t *ctx, block_t *block) {
    exec_t *exec = exec_new(reports, ctx, block);

    while (exec_step(exec)) {
        // empty
    }

    return exec->result;
}
