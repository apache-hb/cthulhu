#include "common/common.h"

#include "std/map.h"
#include "std/vector.h"

#include "std/typed/vector.h"

#include "base/macros.h"
#include "base/panic.h"

typedef struct opt_t {
    reports_t *reports;
    map_t *values; ///< map<ssa_step, ssa_value>
} opt_t;

static const ssa_value_t *fold_operand(opt_t *opt, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandImm: return operand.value;
    case eOperandReg: {
        const ssa_block_t *ctx = operand.vregContext;
        const ssa_step_t *step = typevec_offset(ctx->steps, operand.vregIndex);
        const ssa_value_t *value = map_get_ptr(opt->values, step);
        CTASSERTF(value != NULL, "value for step %p is NULL", step);
        return value;
    }
    default: NEVER("invalid operand kind");
    }
}

static const ssa_value_t *fold_unary(opt_t *opt, const ssa_step_t *step)
{
    ssa_unary_t unary = step->unary;
    const ssa_value_t *val = fold_operand(opt, unary.operand);
    switch (unary.unary)
    {
    case eUnaryNot:
        CTASSERT(val->type->kind == eTypeBool);
        return ssa_value_bool(val->type, !val->boolValue);
    case eUnaryAbs:
        CTASSERT(val->type->kind == eTypeDigit);
        mpz_t result;
        mpz_init(result);
        mpz_abs(result, val->digitValue);
        return ssa_value_digit(val->type, result);
    case eUnaryNeg:
        CTASSERT(val->type->kind == eTypeDigit);
        mpz_init(result);
        mpz_neg(result, val->digitValue);
        return ssa_value_digit(val->type, result);
    case eUnaryFlip:
        CTASSERT(val->type->kind == eTypeDigit);
        mpz_init(result);
        mpz_com(result, val->digitValue);
        return ssa_value_digit(val->type, result);

    default:
        NEVER("invalid unary");
    }
}

static const ssa_value_t *fold_binary(opt_t *opt, const ssa_step_t *step)
{
    ssa_binary_t binary = step->binary;
    const ssa_value_t *lhs = fold_operand(opt, binary.lhs);
    const ssa_value_t *rhs = fold_operand(opt, binary.rhs);

    CTASSERT(lhs->type->kind == rhs->type->kind);
    CTASSERT(lhs->type->kind == eTypeDigit);

    mpz_t result;
    mpz_init(result);
    switch (binary.binary)
    {
    case eBinaryAdd:
        mpz_add(result, lhs->digitValue, rhs->digitValue);
        return ssa_value_digit(lhs->type, result);
    case eBinarySub:
        mpz_sub(result, lhs->digitValue, rhs->digitValue);
        return ssa_value_digit(lhs->type, result);
    case eBinaryMul:
        mpz_mul(result, lhs->digitValue, rhs->digitValue);
        return ssa_value_digit(lhs->type, result);
    case eBinaryDiv:
        mpz_divexact(result, lhs->digitValue, rhs->digitValue);
        return ssa_value_digit(lhs->type, result);
    case eBinaryRem:
        mpz_mod(result, lhs->digitValue, rhs->digitValue);
        return ssa_value_digit(lhs->type, result);

    default:
        NEVER("invalid binary");
    }
}

static const ssa_value_t *fold_load(opt_t *opt, const ssa_step_t *step)
{
    ssa_load_t load = step->load;
    return fold_operand(opt, load.src);
}

static const ssa_value_t *fold_step(opt_t *opt, const ssa_step_t *step)
{
    switch (step->opcode)
    {
    case eOpUnary: return fold_unary(opt, step);
    case eOpBinary: return fold_binary(opt, step);
    case eOpLoad: return fold_load(opt, step);
    default: return NULL;
    }
}

static bool fold_block(opt_t *opt, const ssa_block_t *bb)
{
    bool dirty = false;
    size_t len = typevec_len(bb->steps);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_step_t *step = typevec_offset(bb->steps, i);
        const ssa_value_t *value = fold_step(opt, step);
        if (value != NULL)
        {
            map_set_ptr(opt->values, step, (ssa_value_t*)value);
        }
    }

    return dirty;
}

static bool run_const_fold(opt_t *opt, ssa_symbol_t *symbol)
{
    if (symbol->linkage == eLinkImport) { return false; }

    const ssa_block_t *entry = symbol->entry;
    return fold_block(opt, entry);
}

static void flatten_contents(vector_t **symbols, vector_t *inner)
{
    size_t len = vector_len(inner);
    for (size_t i = 0; i < len; i++)
    {
        ssa_symbol_t *sym = vector_get(inner, i);
        vector_push(symbols, sym);
    }
}

static vector_t *flatten_symbols(vector_t *modules)
{
    vector_t *symbols = vector_new(32);

    size_t len = vector_len(modules);
    for (size_t i = 0; i < len; i++)
    {
        ssa_module_t *mod = vector_get(modules, i);
        flatten_contents(&symbols, mod->globals);
        flatten_contents(&symbols, mod->functions);
    }

    return symbols;
}

void ssa_opt(reports_t *reports, ssa_result_t result)
{
    UNUSED(reports);

    opt_t opt = {
        .reports = reports,
        .values = map_new(32)
    };

    vector_t *symbols = flatten_symbols(result.modules);
    size_t len = vector_len(symbols);
    for (size_t i = 0; i < len; i++)
    {
        ssa_symbol_t *sym = vector_get(symbols, i);
        run_const_fold(&opt, sym);
    }
}
