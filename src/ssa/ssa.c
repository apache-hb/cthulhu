#include "cthulhu/ssa/ssa.h"

#include "cthulhu/hlir/query.h"

typedef struct
{
    map_t *symbols; // map<hlir, flow>
} ssa_emit_t;

static flow_t *flow_new(ssa_emit_t *emit, const hlir_t *symbol)
{
    flow_t *flow = ctu_malloc(sizeof(flow_t));

    flow->name = get_hlir_name(symbol);
    flow->node = get_hlir_node(symbol);
    flow->type = get_hlir_type(symbol);

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

vector_t *ssa_compile(reports_t *reports, vector_t *modules)
{
    size_t totalModules = vector_len(modules);
    vector_t *ssaModules = vector_of(totalModules);

    size_t totalSymbols = calc_total_decls(modules);

    ssa_emit_t emit = {
        .symbols = map_optimal(totalSymbols),
    };

    for (size_t i = 0; i < totalModules; i++)
    {
        const hlir_t *mod = vector_get(modules, i);
        ssa_t *ssa = ssa_new(&emit, mod);
        vector_set(ssaModules, i, ssa);
    }
}
