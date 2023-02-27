#include "cthulhu/hlir/ops.h"
#include "cthulhu/ssa/ssa.h"

#include "report/report.h"
#include "std/vector.h"

#include "std/str.h"
#include "std/set.h"
#include "std/map.h"

#include "base/macros.h"
#include "base/util.h"

#include "common.h"

typedef struct {
    reports_t *reports;
    map_t *globalCache;

    map_t *stepCache;
    operand_t result;
} opt_t;

static operand_t *opt_global(opt_t *opt, const flow_t *flow);
static bool eval_block(opt_t *opt, const flow_t *flow, const block_t *block);

static void set_result(opt_t *opt, const flow_t *flow, operand_t result) 
{
    map_set_ptr(opt->globalCache, flow, BOX(result));
    opt->result = result;
}

static bool get_operand_digit(opt_t *opt, operand_t operand, mpz_t mpz)
{
    if (operand.kind != eOperandDigitImm) 
    {
        return false;
    }

    mpz_set(mpz, operand.mpz);
    return true;
}

static bool eval_binary(opt_t *opt, step_t step, mpz_t result) 
{
    mpz_t lhs;
    mpz_t rhs;

    if (!get_operand_digit(opt, step.lhs, lhs))
    {
        return false;
    }

    if (!get_operand_digit(opt, step.rhs, rhs))
    {
        return false;
    }

    mpz_init_set_ui(result, 0);

    switch (step.binary)
    {
    case eBinaryAdd:
        mpz_add(result, lhs, rhs);
        break;
    case eBinarySub:
        mpz_sub(result, lhs, rhs);
        break;
    case eBinaryMul:
        mpz_mul(result, lhs, rhs);
        break;
    case eBinaryDiv:
        mpz_divexact(result, lhs, rhs);
        break;
    default:
        ctu_assert(opt->reports, "unhandled binary %s", binary_name(step.binary));
        break;
    }

    return true;
}

static bool eval_block(opt_t *opt, const flow_t *flow, const block_t *block)
{
    size_t len = vector_len(block->steps);

    for (size_t i = 0; i < len; i++)
    {
        step_t *step = vector_get(block->steps, i);
        switch (step->opcode)
        {
        case eOpValue:
            map_set_ptr(opt->stepCache, step, &step->value);
            break;

        case eOpReturn:
            set_result(opt, flow, step->value);
            return true;

        case eOpJmp:
            // TODO: unsafe
            if (eval_block(opt, flow, step->label.bb)) {
                return true;
            }
            break;

        case eOpLoad: {
            // TODO: also unsafe
            operand_t *op = opt_global(opt, step->value.flow);
            map_set_ptr(opt->stepCache, step, op);
            break;
        }

        case eOpBinary: {
            
        }

        default:
            ctu_assert(opt->reports, "unhandled opcode %s", ssa_opcode_name(step->opcode));
            break;
        }
    }

    return false;
}

static operand_t *opt_global(opt_t *opt, const flow_t *flow)
{
    operand_t *cached = map_get_ptr(opt->globalCache, flow);
    if (cached != NULL) {
        return cached;
    }

    eval_block(opt, flow, flow->entry);

    operand_t *result = BOX(opt->result);
    map_set_ptr(opt->globalCache, flow, result);

    return result;
}

void opt_module(reports_t *reports, module_t *mod)
{
    section_t symbols = mod->symbols;
    size_t totalGlobals = vector_len(symbols.globals);
    opt_t opt = {
        .reports = reports,
        .globalCache = map_optimal(totalGlobals),
        .stepCache = map_optimal(totalGlobals * 4), // TODO: better estimate
    };

    for (size_t i = 0; i < totalGlobals; i++) 
    {
        flow_t *flow = vector_get(symbols.globals, i);
        opt_global(&opt, flow);
    }
}
