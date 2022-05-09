#include "cthulhu/ssa/ssa.h"

#include "cthulhu/hlir/query.h"

typedef struct
{
    reports_t *reports;
    const ssa_settings_t *settings;

    map_t *modules;
    map_t *flows;
} ssa_emit_t;

static ssa_flow_t *flow_new(const ssa_emit_t *emit, ssa_module_t *parent, const hlir_t *hlir)
{
    ssa_flow_t *flow = ctu_malloc(sizeof(ssa_flow_t));
    
    flow->parent = parent;
    
    flow->node = get_hlir_node(hlir);
    flow->type = get_hlir_type(hlir);

    size_t initialSteps = emit->settings->defaultSteps;

    flow->steps = ctu_malloc(sizeof(ssa_step_t) * initialSteps);
    flow->usedSteps = 0;
    flow->totalSteps = initialSteps;

    return flow;    
}

static ssa_module_t *module_new(const ssa_emit_t *emit, const hlir_t *hlir)
{
    ssa_module_t *module = ctu_malloc(sizeof(ssa_module_t));

    module->moduleName = get_hlir_name(hlir);
    module->node = get_hlir_node(hlir);

    module->modules = map_optimal(32);

    size_t totalFunctions = vector_len(hlir->functions);
    size_t totalGlobals = vector_len(hlir->globals);

    module->functions = map_optimal(totalFunctions);
    module->globals = map_optimal(totalGlobals);

    return module;
}

static void ssa_begin_flow(ssa_emit_t *emit, ssa_module_t *mod, const hlir_t *hlir)
{
    ssa_flow_t *flow = flow_new(emit, mod, hlir);
    map_set_ptr(emit->flows, hlir, flow);
}

static size_t get_total_flows(vector_t *modules)
{
    size_t totalFlows = 0;

    for (size_t i = 0; i < vector_len(modules); i++)
    {
        hlir_t *mod = vector_get(modules, i);
        totalFlows += vector_len(mod->functions);
        totalFlows += vector_len(mod->globals);
    }

    return totalFlows;
}

vector_t *ssa_compile_modules(reports_t *reports, vector_t *modules, const ssa_settings_t *settings)
{
    size_t totalModules = vector_len(modules);

    ssa_emit_t emit = {
        .reports = reports,
        .settings = settings,

        .modules = map_optimal(totalModules),
        .flows = map_optimal(get_total_flows(modules))
    };

    for (size_t i = 0; i < totalModules; i++)
    {
        hlir_t *mod = vector_get(modules, i);
        ssa_module_t *module = module_new(&emit, mod);
        map_set_ptr(emit.modules, mod, module);

        vector_t *globals = mod->globals;
        vector_t *functions = mod->functions;

        size_t totalGlobals = vector_len(globals);
        size_t totalFunctions = vector_len(functions);

        for (size_t j = 0; j < totalGlobals; j++)
        {
            hlir_t *global = vector_get(globals, j);
            ssa_begin_flow(&emit, module, global);
        }

        for (size_t j = 0; j < totalFunctions; j++)
        {
            hlir_t *function = vector_get(functions, j);
            ssa_begin_flow(&emit, module, function);
        }
    }

    for (size_t i = 0; i < totalModules; i++)
    {
        hlir_t *mod = vector_get(modules, i);
        ssa_module_t *module = map_get_ptr(emit.modules, mod);

    }
}
