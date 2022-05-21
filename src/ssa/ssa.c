#include "cthulhu/ssa/ssa.h"

#include "cthulhu/hlir/query.h"

typedef struct
{
    reports_t *reports;
    map_t *symbols; // map<hlir, flow>
} ssa_emit_t;

#define DEFAULT_STEP_LIST_SIZE 16

static flow_t *flow_new(ssa_emit_t *emit, const hlir_t *symbol)
{
    flow_t *flow = ctu_malloc(sizeof(flow_t));

    flow->name = get_hlir_name(symbol);
    flow->node = get_hlir_node(symbol);
    flow->type = get_hlir_type(symbol);

    step_list_t steps = {
        .size = DEFAULT_STEP_LIST_SIZE,
        .used = 0,
        .steps = ctu_malloc(sizeof(step_t) * DEFAULT_STEP_LIST_SIZE),
    };

    flow->steps = steps;

    map_set_ptr(emit->symbols, symbol, flow);

    return flow;
}

static ssa_t *ssa_new(ssa_emit_t *emit, const hlir_t *mod)
{
    ssa_t *ssa = ctu_malloc(sizeof(ssa_t));
    ssa->name = get_hlir_name(mod);
    ssa->node = get_hlir_node(mod);

    size_t totalGlobals = vector_len(mod->globals);
    size_t totalFunctions = vector_len(mod->functions);

    ssa->globals = map_optimal(totalGlobals);
    ssa->functions = map_optimal(totalFunctions);

    for (size_t i = 0; i < totalGlobals; i++)
    {
        const hlir_t *symbol = vector_get(mod->globals, i);
        const char *name = get_hlir_name(symbol);
        flow_t *flow = flow_new(emit, symbol);
        map_set_ptr(ssa->globals, name, flow);
    }

    for (size_t i = 0; i < totalFunctions; i++)
    {
        const hlir_t *symbol = vector_get(mod->functions, i);
        const char *name = get_hlir_name(symbol);
        flow_t *flow = flow_new(emit, symbol);
        map_set_ptr(ssa->functions, name, flow);
    }

    return ssa;
}

static size_t calc_total_decls(vector_t *modules)
{
    size_t total = 0;
    for (size_t i = 0; i < vector_len(modules); i++)
    {
        const hlir_t *mod = vector_get(modules, i);
        total += vector_len(mod->globals);
        total += vector_len(mod->functions);
    }
    return total;
}

static operand_t operand_empty(const hlir_t *hlir)
{
    operand_t operand = {
        .node = get_hlir_node(hlir),
        .kind = OPERAND_EMPTY,
    };

    return operand;
}

static operand_t operand_vreg(const hlir_t *hlir, size_t vreg)
{
    operand_t operand = {
        .node = get_hlir_node(hlir),
        .kind = OPERAND_VREG,
        .vreg = vreg,
    };

    return operand;
}

static size_t add_step(step_list_t *list, const hlir_t *expr, op_t op)
{
    size_t index = list->used;

    if (list->used >= list->size)
    {
        list->size *= 2;
        list->steps = ctu_realloc(list->steps, sizeof(step_t) * list->size);
    }

    step_t step = {
        .type = get_hlir_type(expr),
        .node = get_hlir_node(expr),
        .op = op,
    };

    list->steps[index] = step;

    list->used++;

    return index;
}

static step_t *get_step(step_list_t *list, size_t vreg)
{
    return &list->steps[vreg];
}

static operand_t emit_expr(ssa_emit_t *emit, step_list_t *list, const hlir_t *expr);
static operand_t emit_stmt(ssa_emit_t *emit, step_list_t *list, const hlir_t *stmt);

static operand_t emit_digit_literal(const hlir_t *expr)
{
    operand_t operand = {
        .node = get_hlir_node(expr),
        .kind = OPERAND_CONST,
        .value = expr
    };
    
    return operand;
}

static operand_t emit_binary(ssa_emit_t *emit, step_list_t *list, const hlir_t *expr)
{
    operand_t lhs = emit_expr(emit, list, expr->lhs);
    operand_t rhs = emit_expr(emit, list, expr->rhs);

    size_t vreg = add_step(list, expr, OP_BINARY);
    
    step_t *step = get_step(list, vreg);

    step->binary = expr->binary;
    step->lhs = lhs;
    step->rhs = rhs;

    return operand_vreg(expr, vreg);
}

static operand_t emit_expr(ssa_emit_t *emit, step_list_t *list, const hlir_t *expr)
{
    hlir_kind_t kind = get_hlir_kind(expr);
    switch (kind)
    {
    case HLIR_DIGIT_LITERAL:
        return emit_digit_literal(expr);

    case HLIR_BINARY:
        return emit_binary(emit, list, expr);

    default:
        report(emit->reports, INTERNAL, get_hlir_node(expr), "unhandled expression kind %s", hlir_kind_to_string(kind));
        return operand_empty(expr);
    }
}

static operand_t emit_stmts(ssa_emit_t *emit, step_list_t *list, const hlir_t *stmts)
{
    for (size_t i = 0; i < vector_len(stmts->stmts); i++)
    {
        const hlir_t *stmt = vector_get(stmts->stmts, i);
        emit_stmt(emit, list, stmt);
    }

    return operand_empty(stmts);
}

static operand_t emit_stmt(ssa_emit_t *emit, step_list_t *list, const hlir_t *stmt)
{
    hlir_kind_t kind = get_hlir_kind(stmt);
    switch (kind)
    {
    case HLIR_STMTS:
        return emit_stmts(emit, list, stmt);

    case HLIR_BOOL_LITERAL:
    case HLIR_DIGIT_LITERAL:
    case HLIR_STRING_LITERAL:
        return emit_expr(emit, list, stmt);

    default:
        report(emit->reports, INTERNAL, get_hlir_node(stmt), "unhandled statement kind %s", hlir_kind_to_string(kind));
        return operand_empty(stmt);
    }
}

static void compile_global(ssa_emit_t *emit, const hlir_t *hlir, flow_t *flow)
{
    operand_t op = emit_expr(emit, &flow->steps, hlir->value);
    size_t ret = add_step(&flow->steps, hlir, OP_RETURN);

    step_t *step = get_step(&flow->steps, ret);
    step->operand = op;
}

static void compile_function(ssa_emit_t *emit, const hlir_t *hlir, flow_t *flow)
{
    if (hlir->body == NULL)
    {
        return;
    }
    
    operand_t op = emit_stmt(emit, &flow->steps, hlir->body);
    size_t ret = add_step(&flow->steps, hlir, OP_RETURN);

    step_t *step = get_step(&flow->steps, ret);
    step->operand = op;
}

static void compile_flow(ssa_emit_t *emit, const hlir_t *hlir, flow_t *flow)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case HLIR_GLOBAL:
        compile_global(emit, hlir, flow);
        break;
    case HLIR_FUNCTION:
        compile_function(emit, hlir, flow);
        break;
    
    default:
        report(emit->reports, INTERNAL, get_hlir_node(hlir), "cannot compile flow for %s", hlir_kind_to_string(kind));
        break;
    }
}

static void compile_flows(ssa_emit_t *emit)
{
    map_iter_t iter = map_iter(emit->symbols);

    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        compile_flow(emit, entry.key, entry.value);
        logverbose("compile %s", get_hlir_name(entry.key));
    }
}

vector_t *ssa_compile(reports_t *reports, vector_t *modules)
{
    size_t totalModules = vector_len(modules);
    vector_t *ssaModules = vector_of(totalModules);

    size_t totalSymbols = calc_total_decls(modules);

    ssa_emit_t emit = {
        .reports = reports,
        .symbols = map_optimal(totalSymbols),
    };

    for (size_t i = 0; i < totalModules; i++)
    {
        const hlir_t *mod = vector_get(modules, i);
        ssa_t *ssa = ssa_new(&emit, mod);
        vector_set(ssaModules, i, ssa);
    }

    compile_flows(&emit);

    return ssaModules;
}
