#include "common/common.h"

#include "std/vector.h"
#include "std/map.h"

#include "std/typed/vector.h"

#include "report/report.h"

#include "base/panic.h"

static void add_globals(ssa_module_t *mod, vector_t **vec)
{
    map_iter_t iter = map_iter(mod->globals);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        vector_push(vec, entry.value);
    }
}

static void flatten_globals_inner(vector_t **vec, ssa_module_t *mod)
{
    add_globals(mod, vec);

    map_iter_t modIter = map_iter(mod->modules);
    while (map_has_next(&modIter))
    {
        map_entry_t entry = map_next(&modIter);
        ssa_module_t *mod = entry.value;
        flatten_globals_inner(vec, mod);
    }
}

static vector_t *flatten_globals(ssa_module_t *mod)
{
    vector_t *vec = vector_new(32);
    flatten_globals_inner(&vec, mod);
    return vec;
}

typedef struct opt_t {
    reports_t *reports;

    bool dirty;
    map_t *values; ///< map<ssa_step_t, ssa_value_t>

    const ssa_value_t *result; ///< result of the optimization for the current symbol
} fold_t;

static const ssa_value_t *get_operand_value(fold_t *opt, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandImm: return operand.value;
    case eOperandReg: {
        const ssa_block_t *ctx = operand.vregContext;
        ssa_step_t *step = typevec_offset(ctx->steps, operand.vregIndex);
        return map_get_ptr(opt->values, step);
    }

    default: NEVER("Invalid operand kind: %d", operand.kind);
    }
}

static const ssa_value_t *fold_unary_digit(fold_t *opt, const ssa_type_t *type, unary_t unary, const mpz_t digit)
{
    mpz_t result;

    switch (unary)
    {
    case eUnaryAbs:
        mpz_abs(result, digit);
        break;
    case eUnaryNeg:
        mpz_neg(result, digit);
        break;
    case eUnaryFlip:
        mpz_com(result, digit);
        break;

    default: NEVER("Invalid unary: %s", unary_name(unary));
    }

    return ssa_value_digit(type, result);
}

static const ssa_value_t *fold_unary(fold_t *opt, ssa_unary_t step)
{
    unary_t unary = step.unary;
    const ssa_value_t *value = get_operand_value(opt, step.operand);

    CTASSERT(value != NULL);
    const ssa_type_t *type = value->type;
    switch (type->kind)
    {
    case eTypeBool:
        CTASSERT(unary == eUnaryNot); // nothing else is valid for bool
        return ssa_value_bool(type, !value->boolValue);
    case eTypeDigit:
        return fold_unary_digit(opt, type, unary, value->digitValue);

    default: NEVER("Invalid type: %d", type->kind);
    }
}

static const ssa_value_t *fold_binary(fold_t *opt, ssa_binary_t step)
{
    binary_t binary = step.binary;
    const ssa_value_t *lhs = get_operand_value(opt, step.lhs);
    const ssa_value_t *rhs = get_operand_value(opt, step.rhs);

    // TODO: make sure the types are compatible
    const ssa_type_t *type = lhs->type;
    CTASSERT(type->kind == eTypeDigit);

    mpz_t result;
    switch (binary)
    {
    case eBinaryAdd:
        mpz_add(result, lhs->digitValue, rhs->digitValue);
        break;
    case eBinarySub:
        mpz_sub(result, lhs->digitValue, rhs->digitValue);
        break;
    case eBinaryDiv:
        mpz_tdiv_q(result, lhs->digitValue, rhs->digitValue);
        break;
    case eBinaryMul:
        mpz_mul(result, lhs->digitValue, rhs->digitValue);
        break;
    case eBinaryRem:
        mpz_tdiv_r(result, lhs->digitValue, rhs->digitValue);
        break;

    case eBinaryBitAnd:
        mpz_and(result, lhs->digitValue, rhs->digitValue);
        break;
    case eBinaryBitOr:
        mpz_ior(result, lhs->digitValue, rhs->digitValue);
        break;
    case eBinaryXor:
        mpz_xor(result, lhs->digitValue, rhs->digitValue);
        break;
    case eBinaryShl:
        mpz_mul_2exp(result, lhs->digitValue, mpz_get_ui(rhs->digitValue));
        break;
    case eBinaryShr:
        mpz_tdiv_q_2exp(result, lhs->digitValue, mpz_get_ui(rhs->digitValue));
        break;

    default: NEVER("Invalid binary: %s", binary_name(binary));
    }

    return ssa_value_digit(type, result);
}

static const ssa_value_t *fold_load(fold_t *opt, ssa_load_t step)
{
    ssa_operand_t operand = step.src;
    CTASSERT(operand.kind == eOperandGlobal);

    const ssa_symbol_t *global = operand.global;
    return global->value;
}

static const ssa_value_t *fold_step(fold_t *opt, ssa_step_t *step)
{
    switch (step->opcode)
    {
    case eOpImm: {
        ssa_imm_t imm = step->imm;
        return imm.value;
    }
    case eOpUnary: {
        ssa_unary_t unary = step->unary;
        return fold_unary(opt, unary);
    }
    case eOpBinary: {
        ssa_binary_t binary = step->binary;
        return fold_binary(opt, binary);
    }
    case eOpReturn: {
        ssa_return_t ret = step->ret;
        opt->result = get_operand_value(opt, ret.value);
        return opt->result;
    }
    case eOpLoad: {
        ssa_load_t load = step->load;
        return fold_load(opt, load);
    }

    default: NEVER("Invalid opcode: %d", step->opcode);
    }
}

static const ssa_value_t *fold_block(fold_t *opt, const ssa_block_t *block)
{
    size_t len = typevec_len(block->steps);
    for (size_t i = 0; i < len; i++)
    {
        ssa_step_t *step = typevec_offset(block->steps, i);
        const ssa_value_t *value = fold_step(opt, step);
        if (value == NULL) {
            return NULL;
        }
        map_set_ptr(opt->values, step, (ssa_value_t*)value);
    }

    return opt->result;
}

static void fold_global(fold_t *opt, ssa_symbol_t *global)
{
    if (global->value != NULL) { return; }

    const ssa_value_t *value = fold_block(opt, global->entry);
    if (value == NULL) { return; }

    logverbose("folded global %s", global->name);
    global->value = value;
    opt->dirty = true;
}

static bool run_exec(reports_t *reports, ssa_module_t *mod)
{
    fold_t opt = {
        .reports = reports,
        .values = map_optimal(32)
    };

    vector_t *globals = flatten_globals(mod);
    size_t len = vector_len(globals);
    for (size_t i = 0; i < len; i++)
    {
        ssa_symbol_t *global = vector_get(globals, i);
        fold_global(&opt, global);
    }

    return opt.dirty;
}

void ssa_opt(reports_t *reports, ssa_module_t *mod)
{
    size_t run = 0;
    while (run_exec(reports, mod))
    {
        logverbose("running full pass %zu", ++run);
    }
}
