
typedef struct {
    module_t *mod;

    flow_t *flow;
    size_t ip;

    value_t **values;

    value_t *result;
} state_t;

static void eval_step(state_t *state) {
    step_t *step = step_at(state->flow, state->ip);

    switch (step->type) {
    case OP_EMPTY: break;
    default:
        assert("unknown step eval %d", step->opcode);
        break;
    }
}

value_t *eval_global(module_t *mod, flow_t *flow) {
    
}
