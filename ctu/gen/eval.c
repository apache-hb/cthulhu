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

typedef int(*mpz_fits)(const mpz_t);

static mpz_fits get_fits(const type_t *type) {
    digit_t digit = type->digit;
    switch (digit.kind) {
    case TY_INT: return is_signed(type) ? mpz_fits_sint_p : mpz_fits_uint_p;
    case TY_LONG: case TY_INTPTR: case TY_INTMAX:
        return is_signed(type) ? mpz_fits_slong_p : mpz_fits_ulong_p;
    default: case TY_SHORT: 
        return is_signed(type) ? mpz_fits_sshort_p : mpz_fits_ushort_p;
    }
}

static void check_overflow(reports_t *reports, value_t *value) {
    mpz_fits fits = get_fits(value->type);
    if (!fits(value->digit)) {
        report(reports, ERROR, value->node, "%s overflows type %s", 
            mpz_get_str(NULL, 10, value->digit), 
            type_format(value->type)
        );
    }
}

static value_t *default_value(block_t *block) {
    const type_t *type = block->type;
    const node_t *node = block->node;
    if (is_array(type)) {
        return value_array(type, static_array_length(type));
    }

    return value_of(type, node);
}

static void init_locals(exec_t *exec) {
    block_t *block = exec->block;
    vector_t *all = block->locals;
    size_t len = vector_len(all);

    for (size_t i = 0; i < len; i++) {
        block_t *local = vector_get(all, i);
        value_t *value = default_value(local);
        local->value = value;
    }
}

static exec_t exec_new(world_t *world, block_t *block) {
    size_t size = block->len;

    value_t **values = ctu_malloc(sizeof(value_t*) * size);
    for (size_t i = 0; i < size; i++) {
        values[i] = value_poison("unintalized value");
    }

    exec_t exec = {
        .world = world,
        .block = block,
        .values = values,
        .result = value_empty()
    };

    init_locals(&exec);

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

    return value_digit(v->node, v->type, result);
}

static value_t *value_neg(value_t *v) {
    if (!is_digit(v->type)) {
        return value_poison("not a digit type");
    }

    mpz_t result;
    mpz_neg(result, v->digit);

    return value_digit(v->node, v->type, result);
}

static value_t *value_bitflip(value_t *v) {
    if (!is_digit(v->type)) {
        return value_poison("not a digit type");
    }

    mpz_t result;
    mpz_com(result, v->digit);

    return value_digit(v->node, v->type, result);
}

static value_t *exec_unary(exec_t *exec, step_t step) {
    value_t *operand = get_value(exec, step.operand);

    switch (step.unary) {
    case UNARY_ABS: return value_abs(operand);
    case UNARY_NEG: return value_neg(operand);
    case UNARY_BITFLIP: return value_bitflip(operand);

    default:
        ctu_assert(exec->world->reports, "invalid unary operator");
        return value_poison_with_node("invalid unary operator", step.node);
    }
}

static value_t *exec_binary(exec_t *exec, step_t step) {
    value_t *lhs = get_value(exec, step.lhs);
    value_t *rhs = get_value(exec, step.rhs);
    node_t *node = node_merge(lhs->node, rhs->node);

    mpz_t result;
    mpz_init(result);

    switch (step.binary) {
    case BINARY_ADD:
        mpz_add(result, lhs->digit, rhs->digit);
        return value_digit(node, step.type, result);
    case BINARY_SUB:
        mpz_sub(result, lhs->digit, rhs->digit);
        return value_digit(node, step.type, result);
    case BINARY_MUL:
        mpz_mul(result, lhs->digit, rhs->digit);
        return value_digit(node, step.type, result);
    case BINARY_DIV:
        if (mpz_sgn(rhs->digit) == 0) {
            return value_poison_with_node("division by zero", step.node);
        }
        mpz_tdiv_q(result, lhs->digit, rhs->digit);
        return value_digit(node, step.type, result);
    case BINARY_REM:
        if (mpz_sgn(rhs->digit) == 0) {
            return value_poison_with_node("division by zero", step.node);
        }
        mpz_tdiv_r(result, lhs->digit, rhs->digit);
        return value_digit(node, step.type, result);

    case BINARY_EQ:
        return value_bool(node, mpz_cmp(lhs->digit, rhs->digit) == 0);
    case BINARY_NEQ:
        return value_bool(node, mpz_cmp(lhs->digit, rhs->digit) != 0);
    case BINARY_LT:
        return value_bool(node, mpz_cmp(lhs->digit, rhs->digit) < 0);
    case BINARY_LTE:
        return value_bool(node, mpz_cmp(lhs->digit, rhs->digit) <= 0);
    case BINARY_GT:
        return value_bool(node, mpz_cmp(lhs->digit, rhs->digit) > 0);
    case BINARY_GTE:
        return value_bool(node, mpz_cmp(lhs->digit, rhs->digit) >= 0);

    case BINARY_AND:
        return value_bool(node, lhs->boolean && rhs->boolean);
    case BINARY_OR:
        return value_bool(node, lhs->boolean || rhs->boolean);
    case BINARY_XOR:
        mpz_xor(result, lhs->digit, rhs->digit);
        return value_digit(node, step.type, result);
    case BINARY_SHL:
        mpz_mul_2exp(result, lhs->digit, (mp_bitcnt_t)mpz_size(rhs->digit));
        return value_digit(node, step.type, result);
    case BINARY_SHR:
        mpz_tdiv_q_2exp(result, lhs->digit, (mp_bitcnt_t)mpz_size(rhs->digit));
        return value_digit(node, step.type, result);
    case BINARY_BITAND:
        mpz_and(result, lhs->digit, rhs->digit);
        return value_digit(node, step.type, result);
    case BINARY_BITOR:
        mpz_ior(result, lhs->digit, rhs->digit);
        return value_digit(node, step.type, result);

    default:
        ctu_assert(exec->world->reports, "invalid binary operator %d", step.binary);
        return value_poison_with_node("invalid binary operator", step.node);
    }
}

static value_t *exec_load(exec_t *exec, step_t step) {
    value_t *val = get_value(exec, step.src);
    if (is_array(val->type)) {
        return vector_get(val->elements, val->offset);
    }

    if (is_string(val->type)) {
        return val;
    }

    return eval_block(exec->world, val->block);
}

static value_t *exec_offset(exec_t *exec, step_t step) {
    value_t *base = get_value(exec, step.src);
    value_t *offset = get_value(exec, step.offset);

    if (!is_array(base->type)) {
        printf("%s\n", type_format(base->type));
        return value_poison_with_node("not an array", step.node);
    }

    if (!is_digit(offset->type)) {
        return value_poison_with_node("not an integer", step.node);
    }

    size_t ui = mpz_get_ui(offset->digit) + base->offset;
    return value_offset(base->type, base->elements, ui);
}

/**
 * right now only arrays can generate store instructions
 * in compile time contexts
 */
static void exec_store(exec_t *exec, step_t step) {
    value_t *dst = get_value(exec, step.dst);
    value_t *src = get_value(exec, step.src);

    if (!is_array(dst->type)) {
        report(exec->world->reports, ERROR, step.node, "not an array `%s`", type_format(dst->type));
        return;
    }

    size_t off = dst->offset;
    vector_t *vec = dst->elements;
    if (off > vector_len(vec)) {
        report(exec->world->reports, ERROR, step.node, "out of bounds access %zu is greater than array size of %zu", off, vector_len(vec));
        return;
    }
    
    vector_set(vec, off, src);
}

static value_t *exec_cast(exec_t *exec, step_t step) {
    value_t *src = get_value(exec, step.src);
    value_t *dup = value_dup(src);
    dup->type = step.type;
    return dup;
}

static value_t *exec_return(exec_t *exec, step_t step) {
    value_t *value = get_value(exec, step.operand);
    return value;
}

static bool exec_step(exec_t *exec) {
    size_t here = exec->ip;
    
    if (here >= exec->block->len) {
        return false;
    }

    step_t step = get_step(exec, exec->ip++);

    switch (step.opcode) {
    case OP_RETURN:
        exec->result = exec_return(exec, step);
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

    case OP_OFFSET:
        exec->values[here] = exec_offset(exec, step);
        return true;

    case OP_STORE:
        exec_store(exec, step);
        return true;

    case OP_EMPTY:
    case OP_BLOCK:
        return true;

    case OP_LOAD:
        exec->values[here] = exec_load(exec, step);
        return true;

    case OP_CALL:
        report(exec->world->reports, ERROR, step.node, "can only call `compile` functions at compile time");
        exec->result = value_poison_with_node("invalid function colour", step.node);
        return false;

    case OP_CAST:
        exec->values[here] = exec_cast(exec, step);
        return true;

    default:
        ctu_assert(exec->world->reports, "invalid opcode: %s[%zu] = %d", exec->block->name, exec->ip, step.opcode);
        return false;
    }
}

static value_t *eval_block(world_t *world, block_t *block) {
    if (block->value != NULL) {
        return block->value;
    }

    exec_t exec = exec_new(world, block);

    while (exec_step(&exec)) {
        // empty
    }

    value_t *value = (block->value = exec.result);

    if (is_digit(value->type)) {
        check_overflow(world->reports, value);
    }

    return value;
}

void eval_world(reports_t *reports, module_t *mod) {
    world_t world = { reports, mod };

    size_t nvars = vector_len(mod->vars);
    for (size_t i = 0; i < nvars; i++) {
        block_t *var = vector_get(mod->vars, i);
        eval_block(&world, var);
    }
}
