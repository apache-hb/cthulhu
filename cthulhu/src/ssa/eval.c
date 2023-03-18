#include "cthulhu/hlir/ops.h"
#include "cthulhu/ssa/ssa.h"

#include "report/report.h"

#include "std/vector.h"
#include "std/str.h"
#include "std/set.h"
#include "std/map.h"

#include "base/macros.h"
#include "base/util.h"
#include "base/panic.h"

#include "common.h"

#include <stdio.h>

typedef struct 
{
    reports_t *reports;

    map_t *stepCache;
    map_t *globalCache;

    ssa_step_t *nopStep;

    const ssa_type_t *expectedReturnType;
    const ssa_value_t *result;
    const ssa_flow_t *currentFlow;
} opt_t;

typedef enum {
    eOptSuccess,
    eOptUnsupported,

    eOptTotal
} opt_failure_reason_t;

typedef struct {
    opt_failure_reason_t reason;

    union {
        const ssa_value_t *value;
        const char *detail;
    };
} opt_result_t;

static opt_result_t opt_result_new(opt_failure_reason_t reason)
{
    opt_result_t result = {
        .reason = reason
    };

    return result;
}

static opt_result_t opt_result_value(opt_failure_reason_t reason, const ssa_value_t *value)
{
    opt_result_t result = opt_result_new(reason);
    result.value = value;
    return result;
}

static opt_result_t opt_result_error(opt_failure_reason_t reason, const char *detail)
{
    opt_result_t result = opt_result_new(reason);
    result.detail = detail;
    return result;
}

static opt_result_t opt_ok(const ssa_value_t *value)
{
    return opt_result_value(eOptSuccess, value);
}

static opt_result_t opt_unsupported(const char *reason)
{
    return opt_result_error(eOptUnsupported, reason);
}

static const ssa_value_t *reset_value(opt_t *opt) 
{
    const ssa_value_t *result = opt->result;
    opt->result = NULL;
    return result;
}

static opt_result_t opt_global(opt_t *opt, const ssa_flow_t *flow);
static void build_global(opt_t *opt, ssa_flow_t *flow);

static ssa_value_t *get_operand_value(opt_t *opt, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandImm: return operand.value;
    case eOperandReg: return map_get_ptr(opt->stepCache, operand.vreg);
    case eOperandEmpty: return NULL;

    default:
        report(opt->reports, eInternal, NULL, "unhandled operand %s", ssa_operand_name(operand.kind));
        return NULL;
    }
}

static void set_result(opt_t *opt, const ssa_value_t *value) 
{
    opt->result = value;
}

static opt_result_t opt_load(opt_t *opt, ssa_operand_t src) 
{
    switch (src.kind) 
    {
    case eOperandGlobal: {
        return opt_global(opt, src.global);
    }
    default:
        report(opt->reports, eInternal, NULL, "unhandled operand %s", ssa_operand_name(src.kind));
        return opt_unsupported("unhandled operand");
    }
}

static bool value_is(const ssa_value_t *value, ssa_kind_t kind)
{
    return ssa_get_value_kind(value) == kind;
}

static opt_result_t opt_binary(opt_t *opt, ssa_binary_t binary)
{
    const ssa_value_t *lhs = get_operand_value(opt, binary.lhs);
    const ssa_value_t *rhs = get_operand_value(opt, binary.rhs);

    if (lhs == NULL || rhs == NULL)
    {
        return opt_unsupported("unhandled operand");
    }

    if (!value_is(lhs, eTypeDigit) || !value_is(rhs, eTypeDigit))
    {
        report(opt->reports, eInternal, NULL, "both operands must be ints");
        return opt_unsupported("unsupported operand types");
    }

    mpz_t result;
    mpz_init(result);

    switch (binary.op)
    {
    case eBinaryAdd:
        mpz_add(result, lhs->digit, rhs->digit);
        break;
    case eBinarySub:
        mpz_sub(result, lhs->digit, rhs->digit);
        break;
    case eBinaryMul:
        mpz_mul(result, lhs->digit, rhs->digit);
        break;
    case eBinaryDiv:
        mpz_divexact(result, lhs->digit, rhs->digit);
        break;

    default:
        report(opt->reports, eInternal, NULL, "unhandled binary op %s", binary_name(binary.op));
        return opt_unsupported("unhandled binary op");
    }

    // TODO: lhs->type is a hack here
    return opt_ok(ssa_value_digit_new(result, lhs->type));
}

static opt_result_t opt_unary(opt_t *opt, ssa_unary_t unary)
{
    const ssa_value_t *operand = get_operand_value(opt, unary.operand);

    if (operand == NULL)
    {
        return opt_unsupported("unhandled operand");
    }

    if (!value_is(operand, eTypeDigit))
    {
        report(opt->reports, eInternal, NULL, "operand must be ints");
        return opt_unsupported("unsupported operand types");
    }

    mpz_t result;
    mpz_init(result);

    switch (unary.op)
    {
    case eUnaryAbs:
        mpz_abs(result, operand->digit);
        break;
    case eUnaryNeg:
        mpz_neg(result, operand->digit);
        break;
    
    default:
        report(opt->reports, eInternal, NULL, "unhandled unary op %s", unary_name(unary.op));
        return opt_unsupported("unhandled unary op");
    }

    return opt_ok(ssa_value_digit_new(result, operand->type));
}

static opt_result_t opt_cast(opt_t *opt, ssa_cast_t cast)
{
    const ssa_value_t *operand = get_operand_value(opt, cast.operand);
    const ssa_type_t *srcType = ssa_get_operand_type(opt->reports, opt->currentFlow, cast.operand);
    const ssa_type_t *dstType = cast.type;

    if (srcType->kind == eTypeDigit && dstType->kind == eTypeOpaque)
    {
        if (srcType->digit == eDigitPtr)
        {
            return opt_ok(ssa_value_ptr_new(dstType, operand->digit));
        }
    }

    return opt_result_error(eOptUnsupported, "invalid cast");
}

static opt_result_t opt_step(opt_t *opt, const ssa_step_t *step, bool *result)
{
    ssa_opcode_t kind = step->opcode;
    switch (kind)
    {
    case eOpReturn: {
        ssa_return_t ret = step->ret;
        *result = true;
        return opt_ok(get_operand_value(opt, ret.value));
    }

    case eOpLoad: {
        ssa_load_t load = step->load;
        return opt_load(opt, load.src);
    }

    case eOpBinary: {
        ssa_binary_t binary = step->binary;
        return opt_binary(opt, binary);
    }

    case eOpUnary: {
        ssa_unary_t unary = step->unary;
        return opt_unary(opt, unary);
    }

    case eOpCast: {
        ssa_cast_t cast = step->cast;
        return opt_cast(opt, cast);
    }

    default:
        report(opt->reports, eInternal, NULL, "unhandled opcode %s", ssa_opcode_name(kind));
        return opt_unsupported("unhandled opcode");
    }
}

static bool opt_block(opt_t *opt, const ssa_block_t *block)
{
    size_t steps = vector_len(block->steps);

    for (size_t i = 0; i < steps; i++)
    {
        bool shouldReturn = false;
        ssa_step_t *step = vector_get(block->steps, i);
        opt_result_t result = opt_step(opt, step, &shouldReturn);

        if (result.reason != eOptSuccess)
        {
            // failed to optimize
            report(opt->reports, eInternal, NULL, "failed to optimize step");
            break;
        }

        if (shouldReturn) 
        {
            set_result(opt, result.value);
            return true;
        }

        map_set_ptr(opt->stepCache, step, (void*)result.value);
    }

    set_result(opt, NULL);
    return false;
}

static opt_result_t opt_global(opt_t *opt, const ssa_flow_t *flow)
{
    const ssa_value_t *value = map_get_ptr(opt->globalCache, flow);
    if (value != NULL)
    {
        return opt_ok(value);
    }
    
    if (!opt_block(opt, flow->entry))
    {
        report(opt->reports, eInternal, NULL, "failed to optimize block");
        return opt_unsupported("failed to optimize block");
    }

    const ssa_value_t *result = reset_value(opt);

    map_set_ptr(opt->globalCache, flow, (void*)result);
    return opt_ok(result);
}

static void build_global(opt_t *opt, ssa_flow_t *flow)
{
    // if there is no entry then the global is uninitialized
    if (flow->linkage == eLinkImported)
    {
        return;
    }

    opt->expectedReturnType = flow->type;
    opt->currentFlow = flow;
    opt_result_t result = opt_global(opt, flow);
    
    if (result.reason != eOptSuccess)
    {
        report(opt->reports, eInternal, NULL, "failed to optimize global");
        flow->value = ssa_value_empty_new(flow->type);
    }
    else
    {
        flow->value = result.value;
    }
}

static void build_function(opt_t *opt, ssa_flow_t *flow)
{
    UNUSED(opt);
    UNUSED(flow);
}

void ssa_opt_module(reports_t *reports, ssa_module_t *mod)
{
    section_t symbols = mod->symbols;
    size_t totalGlobals = vector_len(symbols.globals);
    size_t totalFunctions = vector_len(symbols.functions);

    ssa_step_t nopStep = {
        .opcode = eOpNop,
        .id = "nop"
    };

    opt_t opt = {
        .reports = reports,
        .stepCache = map_new(0x1000), // TODO: better size
        .globalCache = map_new(0x1000), // TODO: better size

        .nopStep = BOX(nopStep),

        .expectedReturnType = NULL,
        .result = NULL
    };

    for (size_t i = 0; i < totalGlobals; i++)
    {
        ssa_flow_t *flow = vector_get(symbols.globals, i);
        build_global(&opt, flow);
    }

    for (size_t i = 0; i < totalFunctions; i++)
    {
        ssa_flow_t *flow = vector_get(symbols.functions, i);
        build_function(&opt, flow);
    }
}
