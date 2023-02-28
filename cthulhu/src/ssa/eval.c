#include "cthulhu/hlir/ops.h"
#include "cthulhu/ssa/ssa.h"

#include "mini-gmp.h"
#include "report/report.h"
#include "src/ssa/common.h"
#include "std/vector.h"

#include "std/str.h"
#include "std/set.h"
#include "std/map.h"

#include "base/macros.h"
#include "base/util.h"

#include "common.h"

#include <stdio.h>

typedef struct 
{
    reports_t *reports;

    map_t *stepCache;
    map_t *globalCache;

    const ssa_value_t *result;
} opt_t;

static const ssa_value_t *reset_value(opt_t *opt) 
{
    const ssa_value_t *result = opt->result;
    opt->result = NULL;
    return result;
}

static const ssa_value_t *opt_global(opt_t *opt, const ssa_flow_t *flow);
static void build_global(opt_t *opt, ssa_flow_t *flow);

static const ssa_value_t *get_operand_value(opt_t *opt, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandImm: return operand.value;
    case eOperandReg: return map_get_ptr(opt->stepCache, operand.vreg);

    default:
        report(opt->reports, eInternal, NULL, "unhandled operand %s", ssa_operand_name(operand.kind));
        return NULL;
    }
}

static void set_result(opt_t *opt, ssa_operand_t operand) 
{
    opt->result = get_operand_value(opt, operand);
}

static void opt_load(opt_t *opt, const ssa_step_t *step, ssa_operand_t src) 
{
    switch (src.kind) 
    {
    case eOperandGlobal: {
        map_set_ptr(opt->stepCache, step, (void*)opt_global(opt, src.global));
        break;
    }
    default:
        report(opt->reports, eInternal, NULL, "unhandled operand %s", ssa_operand_name(src.kind));
        break;
    }
}

static bool value_is(const ssa_value_t *value, ssa_kind_t kind)
{
    return ssa_get_value_kind(value) == kind;
}

static void opt_binary(opt_t *opt, const ssa_step_t *step, ssa_binary_t binary)
{
    const ssa_value_t *lhs = get_operand_value(opt, binary.lhs);
    const ssa_value_t *rhs = get_operand_value(opt, binary.rhs);

    if (lhs == NULL || rhs == NULL)
    {
        report(opt->reports, eInternal, NULL, "unhandled binary operands");
        return;
    }

    if (!value_is(lhs, eTypeDigit) || !value_is(rhs, eTypeDigit))
    {
        report(opt->reports, eInternal, NULL, "both operands must be ints");
        return;
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
        return;
    }

    // TODO: type is a hack here
    map_set_ptr(opt->stepCache, step, value_digit_new(result, lhs->type));
}

static bool opt_block(opt_t *opt, const ssa_block_t *block)
{
    size_t steps = vector_len(block->steps);

    for (size_t i = 0; i < steps; i++)
    {
        ssa_step_t *step = vector_get(block->steps, i);
        ssa_opcode_t kind = step->opcode;

        switch (kind)
        {
        case eOpReturn: {
            ssa_return_t ret = step->ret;
            set_result(opt, ret.value);
            return true;
        }

        case eOpLoad: {
            ssa_load_t load = step->load;
            opt_load(opt, step, load.src);
            break;
        }

        case eOpBinary: {
            ssa_binary_t binary = step->binary;
            opt_binary(opt, step, binary);
            break;
        }

        default:
            report(opt->reports, eInternal, NULL, "unhandled opcode %s", ssa_opcode_name(kind));
            return false;
        }
    }

    return false;
}

static const ssa_value_t *opt_global(opt_t *opt, const ssa_flow_t *flow)
{
    const ssa_value_t *value = map_get_ptr(opt->globalCache, flow);
    if (value != NULL)
    {
        return value;
    }
    
    if (!opt_block(opt, flow->entry))
    {
        report(opt->reports, eInternal, NULL, "failed to optimize block");
        return NULL;
    }

    const ssa_value_t *result = reset_value(opt);

    map_set_ptr(opt->globalCache, flow, (void*)result);
    return result;
}

static void build_global(opt_t *opt, ssa_flow_t *flow)
{
    flow->value = opt_global(opt, flow);
}

void ssa_opt_module(reports_t *reports, ssa_module_t *mod)
{
    section_t symbols = mod->symbols;
    size_t totalGlobals = vector_len(symbols.globals);

    opt_t opt = {
        .reports = reports,
        .stepCache = map_new(0x1000), // TODO: better size
        .globalCache = map_new(0x1000), // TODO: better size
        .result = NULL
    };

    for (size_t i = 0; i < totalGlobals; i++)
    {
        ssa_flow_t *flow = vector_get(symbols.globals, i);
        printf("build: %s\n", flow->name);
        build_global(&opt, flow);
    }
}
