#include "cthulhu/ssa/ssa.h"

#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"

#include "base/memory.h"

#include "report/report.h"
#include "std/map.h"

typedef struct
{
    reports_t *reports;
    module_t *mod;
    map_t *flows;
    flow_t *current;
} ssa_t;

static size_t total_flows(vector_t *modules)
{
    size_t total = 0;
    for (size_t i = 0; i < vector_len(modules); i++)
    {
        const hlir_t *mod = vector_get(modules, i);
        total += vector_len(mod->functions);
    }
    return total;
}

static void forward_flows(map_t *cache, const hlir_t *hlir)
{
    size_t totalFunctions = vector_len(hlir->functions);

    for (size_t i = 0; i < totalFunctions; i++)
    {
        const hlir_t *function = vector_get(hlir->functions, i);
        flow_t *flow = ctu_malloc(sizeof(flow_t));
        map_set_ptr(cache, function, flow);
    }
}

static vreg_t add_step(ssa_t *ssa, step_t step)
{
    flow_t *flow = ssa->current;
    if (flow->total >= flow->len)
    {
        flow->total += 16;
        flow->steps = ctu_realloc(flow->steps, sizeof(step_t) * flow->total);
    }

    flow->steps[flow->len] = step;
    return flow->len++;
}

static step_t new_step(opcode_t op, type_t type)
{
    step_t step = {.op = op, .type = type};

    return step;
}

static operand_t operand_empty(void)
{
    operand_t operand = {.kind = eOperandEmpty};
    return operand;
}

static operand_t operand_vreg(vreg_t reg)
{
    operand_t operand = {.kind = eOperandReg, .reg = reg};
    return operand;
}

static operand_t operand_digit(const mpz_t digit, digit_t width, sign_t sign)
{
    value_t value = {.type = {.kind = eLiteralInt, .digitKind = {.width = width, .sign = sign}}};

    mpz_init_set(value.digit, digit);

    operand_t operand = {.kind = eOperandImm, .value = value};

    return operand;
}

static operand_t emit_hlir(ssa_t *ssa, const hlir_t *hlir);

static operand_t emit_digit(const hlir_t *hlir)
{
    return operand_digit(hlir->digit, hlir->width, hlir->sign);
}

static operand_t emit_binary(ssa_t *ssa, const hlir_t *hlir)
{
    operand_t lhs = emit_hlir(ssa, hlir->lhs);
    operand_t rhs = emit_hlir(ssa, hlir->rhs);

    // TODO: convert hlir type to type_t
    type_t type = {.kind = eLiteralInt, .digitKind = {.width = eInt, .sign = eSigned}};

    step_t step = new_step(eOpBinary, type);
    step.binary = hlir->binary;
    step.lhs = lhs;
    step.rhs = rhs;

    return operand_vreg(add_step(ssa, step));
}

static operand_t emit_compare(ssa_t *ssa, const hlir_t *hlir)
{
    operand_t lhs = emit_hlir(ssa, hlir->lhs);
    operand_t rhs = emit_hlir(ssa, hlir->rhs);

    type_t type = {.kind = eLiteralBool};
    step_t step = new_step(eOpCompare, type);
    step.compare = hlir->compare;
    step.lhs = lhs;
    step.rhs = rhs;

    return operand_vreg(add_step(ssa, step));
}

static operand_t emit_return(ssa_t *ssa, const hlir_t *hlir)
{
    operand_t result = emit_hlir(ssa, hlir->result);

    type_t type = {.kind = eLiteralEmpty};
    step_t step = new_step(eOpReturn, type);
    step.result = result;

    return operand_vreg(add_step(ssa, step));
}

static operand_t emit_hlir(ssa_t *ssa, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirLiteralDigit:
        return emit_digit(hlir);

    case eHlirBinary:
        return emit_binary(ssa, hlir);

    case eHlirCompare:
        return emit_compare(ssa, hlir);

    case eHlirReturn:
        return emit_return(ssa, hlir);

    default:
        report(ssa->reports, eInternal, get_hlir_node(hlir), "unexpected hlir %s", hlir_kind_to_string(kind));
        return operand_empty();
    }
}

static void compile_flow(ssa_t *ssa, flow_t *flow, const hlir_t *hlir)
{
    ssa->current = flow;

    if (hlir->body != NULL)
    {
        flow->len = 0;
        flow->total = 16;
        flow->steps = ctu_malloc(sizeof(step_t) * 16);
        emit_hlir(ssa, hlir->body);
    }
    else
    {
        flow->len = 0;
        flow->total = 0;
        flow->steps = NULL;
    }

    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);

    module_t *mod = ssa->mod;
    switch (attribs->linkage)
    {
    case eLinkEntryCli:
        mod->cliEntry = flow;
        break;
    case eLinkEntryGui:
        mod->guiEntry = flow;
        break;
    case eLinkExported:
        vector_push(&mod->exports, flow);
        break;
    case eLinkImported:
        vector_push(&mod->imports, flow);
        break;
    case eLinkInternal:
        vector_push(&mod->internals, flow);
        break;

    default:
        break;
    }
}

static void compile_flows(reports_t *reports, module_t *mod, map_t *flows)
{
    ssa_t ssa = {.reports = reports, .mod = mod, .flows = flows};

    map_iter_t iter = map_iter(flows);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        compile_flow(&ssa, entry.value, entry.key);
    }
}

module_t *ssa_compile(reports_t *reports, vector_t *modules)
{
    size_t totalFlows = total_flows(modules);
    size_t len = vector_len(modules);

    module_t *mod = ctu_malloc(sizeof(module_t));
    mod->imports = vector_new(totalFlows);
    mod->exports = vector_new(totalFlows);
    mod->internals = vector_new(totalFlows);

    map_t *flows = map_optimal(totalFlows);

    for (size_t i = 0; i < len; i++)
    {
        const hlir_t *hlir = vector_get(modules, i);
        forward_flows(flows, hlir);
    }

    compile_flows(reports, mod, flows);

    return mod;
}
