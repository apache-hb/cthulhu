#include "emit.h"

#include "ctu/util/util.h"

static size_t add_step(flow_t *flow, step_t step) {
    if (flow->len + 1 >= flow->size) {
        flow->size += 16;
        flow->steps = ctu_realloc(flow->steps, flow->size * sizeof(step_t));
    }

    flow->steps[flow->len + 1] = step;
    return flow->len++;
}

static flow_t *build_flow(lir_t *lir) {
    (void)add_step;
    flow_t *flow = ctu_malloc(sizeof(flow_t));
    flow->name = lir->name;
    flow->value = NULL;

    flow->len = 0;
    flow->size = 16;
    flow->steps = ctu_malloc(sizeof(step_t) * flow->size);

    return flow;
}

module_t *module_build(lir_t *root) {
    vector_t *vars = root->vars;
    size_t len = vector_len(vars);

    vector_t *flows = vector_new(len);
    for (size_t i = 0; i < len; i++) {
        lir_t *var = vector_get(vars, i);
        flow_t *flow = build_flow(var);
        vector_push(&flows, flow);
    }

    module_t *mod = ctu_malloc(sizeof(module_t));
    mod->flows = flows;
    return mod;
}
