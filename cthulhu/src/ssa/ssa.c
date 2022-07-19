#include "cthulhu/ssa/ssa.h"

#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"

#include "base/memory.h"
#include "base/panic.h"

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
    if (flow->stepsSize >= flow->stepsLen)
    {
        flow->stepsSize += 16;
        flow->steps = ctu_realloc(flow->steps, sizeof(step_t) * flow->stepsSize);
    }

    flow->steps[flow->stepsLen] = step;
    return flow->stepsLen++;
}

static type_t new_type(literal_t kind)
{
    type_t type = {.kind = kind};

    return type;
}

static type_t invalid_type(void)
{
    return new_type(eLiteralEmpty);
}

static type_t digit_type(const hlir_t *hlir)
{
    CTASSERT(hlir_is(hlir, eHlirDigit));

    type_t type = new_type(eLiteralInt);
    type.digitKind.sign = hlir->sign;
    type.digitKind.width = hlir->width;
    return type;
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

static operand_t operand_ref(flow_t *flow)
{
    operand_t operand = {.kind = eOperandRef, .ref = flow};
    return operand;
}

static operand_t operand_digit(const hlir_t *hlir)
{
    CTASSERT(hlir_is(hlir, eHlirLiteralDigit));

    value_t value = {.type = digit_type(get_hlir_type(hlir))};

    mpz_init_set(value.digit, hlir->digit);

    operand_t operand = {.kind = eOperandImm, .value = value};

    return operand;
}

static type_t emit_type(ssa_t *ssa, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirDigit:
        return digit_type(hlir);
    case eHlirBool:
        return new_type(eLiteralBool);
    case eHlirVoid:
        return new_type(eLiteralVoid);
    case eHlirString:
        return new_type(eLiteralString);

    default:
        report(ssa->reports, eInternal, get_hlir_node(hlir), "unsupported type %s", hlir_kind_to_string(kind));
        return invalid_type();
    }
}

static operand_t emit_hlir(ssa_t *ssa, const hlir_t *hlir);

static operand_t emit_digit(const hlir_t *hlir)
{
    return operand_digit(hlir);
}

static operand_t emit_binary(ssa_t *ssa, const hlir_t *hlir)
{
    operand_t lhs = emit_hlir(ssa, hlir->lhs);
    operand_t rhs = emit_hlir(ssa, hlir->rhs);

    type_t type = emit_type(ssa, get_hlir_type(hlir));
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

    type_t type = new_type(eLiteralBool);
    step_t step = new_step(eOpCompare, type);
    step.compare = hlir->compare;
    step.lhs = lhs;
    step.rhs = rhs;

    return operand_vreg(add_step(ssa, step));
}

static operand_t emit_return(ssa_t *ssa, const hlir_t *hlir)
{
    operand_t result = emit_hlir(ssa, hlir->result);

    type_t type = invalid_type();
    step_t step = new_step(eOpReturn, type);
    step.result = result;

    return operand_vreg(add_step(ssa, step));
}

static operand_t emit_call(ssa_t *ssa, const hlir_t *hlir)
{
    operand_t *args = NULL;

    size_t len = vector_len(hlir->args);
    if (len > 0)
    {
        args = ctu_malloc(sizeof(operand_t) * len);

        for (size_t i = 0; i < len; i++)
        {
            const hlir_t *arg = vector_get(hlir->args, i);
            args[i] = emit_hlir(ssa, arg);
        }
    }

    operand_t func = emit_hlir(ssa, hlir->call);

    type_t type = emit_type(ssa, closure_result(hlir->call));
    step_t step = new_step(eOpCall, type);
    step.call = func;
    step.count = len;
    step.args = args;

    return operand_vreg(add_step(ssa, step));
}

static operand_t emit_ref(ssa_t *ssa, const hlir_t *hlir)
{
    flow_t *flow = map_get_ptr(ssa->flows, hlir);
    CTASSERT(flow != NULL);

    return operand_ref(flow);
}

static operand_t emit_stmts(ssa_t *ssa, const hlir_t *hlir)
{
    for (size_t i = 0; i < vector_len(hlir->stmts); i++)
    {
        const hlir_t *stmt = vector_get(hlir->stmts, i);
        emit_hlir(ssa, stmt);
    }

    return operand_empty();
}

static operand_t emit_assign(ssa_t *ssa, const hlir_t *hlir)
{
    operand_t src = emit_hlir(ssa, hlir->src);
    operand_t dst = emit_hlir(ssa, hlir->dst);

    step_t step = new_step(eOpStore, new_type(eLiteralEmpty));
    step.dst = dst;
    step.src = src;

    return operand_vreg(add_step(ssa, step));
}

static operand_t emit_name(ssa_t *ssa, const hlir_t *hlir)
{
    operand_t src = emit_hlir(ssa, hlir->read);
    step_t step = new_step(eOpLoad, emit_type(ssa, get_hlir_type(hlir)));
    step.src = src;

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

    case eHlirCall:
        return emit_call(ssa, hlir);

    case eHlirFunction:
        return emit_ref(ssa, hlir);

    case eHlirStmts:
        return emit_stmts(ssa, hlir);

    case eHlirAssign:
        return emit_assign(ssa, hlir);

    case eHlirName:
        return emit_name(ssa, hlir);

    default:
        report(ssa->reports, eInternal, get_hlir_node(hlir), "unexpected hlir %s", hlir_kind_to_string(kind));
        return operand_empty();
    }
}

static void compile_flow(ssa_t *ssa, flow_t *flow, const hlir_t *hlir)
{
    ssa->current = flow;

    // TODO: init function params

    flow->name = get_hlir_name(hlir);
    if (flow->name == NULL)
    {
        flow->name = "<unnamed>";
    }
    
    flow->stepsLen = 0;

    if (hlir->body != NULL)
    {
        flow->stepsSize = 16;

        flow->steps = ctu_malloc(sizeof(step_t) * 16);
        flow->locals = ctu_malloc(sizeof(type_t) * 4);
        emit_hlir(ssa, hlir->body);

        flow->locals = vector_new(16);
    }
    else
    {
        flow->stepsSize = 0;

        flow->steps = NULL;
        flow->locals = NULL;
    }

    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);

    module_t *mod = ssa->mod;
    switch (attribs->linkage)
    {
    case eLinkEntryCli:
        CTASSERT(mod->cliEntry == NULL);
        mod->cliEntry = flow;
        break;
    case eLinkEntryGui:
        CTASSERT(mod->guiEntry == NULL);
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
    ssa_t ssa = {.reports = reports, .mod = mod, .flows = flows,};

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
