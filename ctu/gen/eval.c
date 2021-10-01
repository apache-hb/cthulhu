#include "eval.h"

typedef struct {
    reports_t *reports;
    module_t *mod;
} world_t;

typedef struct {
    world_t *world;
    block_t *block;
    size_t ip;

    value_t **values;
    value_t *result;
} exec_t;

static exec_t *exec_new(world_t *world, block_t *block) {
    size_t size = block->len;

    value_t **values = ctu_malloc(sizeof(value_t*) * size);
    for (size_t i = 0; i < size; i++) {
        values[i] = value_poison("unintalized value");
    }

    exec_t *exec = ctu_malloc(sizeof(exec_t));
    exec->world = world;
    exec->block = block;
    exec->ip = 0;
    exec->values = values;
    exec->result = value_poison("unintalized result");

    return exec;
}

static value_t *eval_block(world_t *world, block_t *block);

static step_t get_step(exec_t *exec, size_t ip) {
    return exec->block->steps[ip];
}

static value_t *get_value(exec_t *exec, operand_t it) {
    switch (it.kind) {
    case IMM: return it.imm;
    case VREG: return exec->values[it.vreg];
    case ADDRESS: return value_block(it.block);
    case EMPTY: return value_empty();

    default:
        ctu_assert(exec->world->reports, "invalid operand kind");
        return value_poison("invalid operand kind");
    }
}

static size_t get_label(exec_t *exec, operand_t it) {
    if (it.kind != LABEL) {
        ctu_assert(exec->world->reports, "invalid operand kind");
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
        ctu_assert(exec->world->reports, "invalid unary operator");
        return value_poison("invalid unary operator");
    }
}

static value_t *exec_binary(exec_t *exec, step_t step) {
    value_t *lhs = get_value(exec, step.lhs);
    value_t *rhs = get_value(exec, step.rhs);

    mpz_t result;
    mpz_init(result);

    switch (step.binary) {
    case BINARY_ADD:
        mpz_add(result, lhs->digit, rhs->digit);
        return value_digit(step.type, result);
    case BINARY_SUB:
        mpz_sub(result, lhs->digit, rhs->digit);
        return value_digit(step.type, result);
    case BINARY_MUL:
        mpz_mul(result, lhs->digit, rhs->digit);
        return value_digit(step.type, result);
    case BINARY_DIV:
        if (mpz_sgn(rhs->digit) == 0) {
            return value_poison("division by zero");
        }
        mpz_tdiv_q(result, lhs->digit, rhs->digit);
        return value_digit(step.type, result);
    case BINARY_REM:
        if (mpz_sgn(rhs->digit) == 0) {
            return value_poison("division by zero");
        }
        mpz_tdiv_r(result, lhs->digit, rhs->digit);
        return value_digit(step.type, result);

    case BINARY_EQ:
        return value_bool(mpz_cmp(lhs->digit, rhs->digit) == 0);
    case BINARY_NEQ:
        return value_bool(mpz_cmp(lhs->digit, rhs->digit) != 0);
    case BINARY_LT:
        return value_bool(mpz_cmp(lhs->digit, rhs->digit) < 0);
    case BINARY_LTE:
        return value_bool(mpz_cmp(lhs->digit, rhs->digit) <= 0);
    case BINARY_GT:
        return value_bool(mpz_cmp(lhs->digit, rhs->digit) > 0);
    case BINARY_GTE:
        return value_bool(mpz_cmp(lhs->digit, rhs->digit) >= 0);

    case BINARY_AND:
    case BINARY_OR:
    case BINARY_XOR:
    case BINARY_SHL:
    case BINARY_SHR:
    case BINARY_BITAND:
    case BINARY_BITOR:

    default:
        ctu_assert(exec->world->reports, "invalid binary operator %d", step.binary);
        return value_poison("invalid binary operator");
    }
}

static value_t *exec_load(exec_t *exec, step_t step) {
    value_t *val = get_value(exec, step.src);
    return eval_block(exec->world, val->block);
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

    case OP_BINARY:
        exec->values[here] = exec_binary(exec, step);
        return true;

    case OP_JMP:
        exec->ip = get_label(exec, step.label);
        return exec->ip != SIZE_MAX;

    case OP_EMPTY: 
    case OP_BLOCK:
        return true;

    case OP_LOAD:
        exec->values[here] = exec_load(exec, step);
        return true;

    default:
        ctu_assert(exec->world->reports, "%s invalid opcode: [%zu] = %d", exec->block->name, exec->ip, step.opcode);
        return false;
    }
}

static value_t *eval_block(world_t *world, block_t *block) {
    if (block->value != NULL) {
        return block->value;
    }

    exec_t *exec = exec_new(world, block);

    while (exec_step(exec)) {
        // empty
    }

    block->value = exec->result;

    return exec->result;
}

void eval_world(reports_t *reports, module_t *mod) {
    world_t world = { reports, mod };

    size_t nvars = vector_len(mod->vars);
    for (size_t i = 0; i < nvars; i++) {
        block_t *var = vector_get(mod->vars, i);
        eval_block(&world, var);
    }
}
