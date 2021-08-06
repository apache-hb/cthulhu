#include "eval.h"

#include "ctu/util/report.h"

typedef struct {
    module_t *mod;

    flow_t *flow;
    size_t ip;

    value_t **values;

    value_t **args;

    value_t *result;

    bool error;
} state_t;

static value_t *build_value(type_t *type) {
    value_t *value = ctu_malloc(sizeof(value_t));

    value->type = type;

    if (type && is_integer(type)) {
        mpz_init(value->digit);
    }

    return value;
}

value_t *empty_value(void) {
    return build_value(NULL);
}

static step_t *current(state_t *state) {
    return &state->flow->steps[state->ip];
}

static reportid_t step_report(level_t level, state_t *state, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    step_t *here = current(state);
    reportid_t id = reportv(level, here->source, here->where, fmt, args);
    va_end(args);
    return id;
}

static void sanitize_value(state_t *state, value_t *value) {
    if (value->type && is_integer(value->type)) {
        step_t *here = current(state);
        sanitize_range(value->type, value->digit, here->source, here->where);
    }
}

static bool is_uninit(value_t *value) {
    return value->type == NULL;
}

static bool report_uninit(state_t *state, value_t *value) {
    if (is_uninit(value)) {
        state->error = true;
        step_t *step = current(state);
        report(LEVEL_ERROR, step->source, step->where, "uninitialized value accessed");
        return true;
    }

    return false;
}

static value_t *imm_value(imm_t imm) {
    value_t *value = build_value(imm.type);

    if (is_boolean(imm.type)) {
        value->boolean = imm.b;
    } else if (is_integer(imm.type)) {
        mpz_init_set(value->digit, imm.num);
    } else {
        assert("unknown imm type");
    }

    return value;
}

static value_t *func_value(module_t *mod, size_t idx) {
    flow_t *flow = mod->flows + idx;
    type_t *type = get_resolved_type(flow->node);

    value_t *value = build_value(type);
    value->func = flow;

    return value;
}

static value_t *make_value(state_t *state, operand_t op) {
    value_t *value = empty_value();
    switch (op.kind) {
    case ARG: 
        value = state->args[op.arg]; 
        break;

    case VREG: 
        value = state->values[op.vreg]; 
        break;

    case IMM: 
        return imm_value(op.imm);

    case VAR:
        value = eval_global(state->mod, state->mod->vars + op.var);
        break;

    case FUNC:
        value = func_value(state->mod, op.func);
        break;

    default:
        assert("make-value %d", op.kind);
        break;
    }

    report_uninit(state, value);

    return value;
}

static bool get_cond(state_t *state, operand_t op) {
    value_t *value = make_value(state, op);
    
    if (report_uninit(state, value)) {
        return false;
    }

    ASSERT(is_boolean(value->type))("get-cond require a boolean");

    return value->boolean;
}

static size_t get_dst(operand_t op) {
    /* for now we only jump to blocks, later we may need more */
    ASSERT(op.kind == BLOCK)("can only jump to blocks");

    return op.label;
}

static value_t *eval_binary(state_t *state, type_t *type, binary_t binary, operand_t left, operand_t right) {
    value_t *value = build_value(type);
    value_t *lhs = make_value(state, left);
    value_t *rhs = make_value(state, right);

    switch (binary) {
    case BINARY_ADD:
        mpz_add(value->digit, lhs->digit, rhs->digit);
        break;
    case BINARY_SUB:
        mpz_sub(value->digit, lhs->digit, rhs->digit);
        break;
    case BINARY_MUL:
        mpz_mul(value->digit, lhs->digit, rhs->digit);
        break;
    case BINARY_DIV:
        if (mpz_sgn(rhs->digit) == 0) {
            step_report(LEVEL_ERROR, state, "divide by zero");
        }
        mpz_div(value->digit, lhs->digit, rhs->digit);
        break;
    case BINARY_REM:
        if (mpz_sgn(rhs->digit) == 0) {
            step_report(LEVEL_ERROR, state, "remainder by zero");
        }
        mpz_mod(value->digit, lhs->digit, rhs->digit);
        break;

    case BINARY_AND:
        value->boolean = lhs->boolean && rhs->boolean;
        break;
    case BINARY_OR:
        value->boolean = lhs->boolean || rhs->boolean;
        break;

    case BINARY_XOR:
        mpz_xor(value->digit, lhs->digit, rhs->digit);
        break;

    case BINARY_BITOR:
        mpz_ior(value->digit, lhs->digit, rhs->digit);
        break;
    case BINARY_BITAND:
        mpz_and(value->digit, lhs->digit, rhs->digit);
        break;
    case BINARY_EQ:
        value->boolean = mpz_cmp(lhs->digit, rhs->digit) == 0;
        break;
    case BINARY_NEQ:
        value->boolean = mpz_cmp(lhs->digit, rhs->digit) != 0;
        break;
    case BINARY_LT:
        value->boolean = mpz_cmp(lhs->digit, rhs->digit) < 0;
        break;
    case BINARY_GT:
        value->boolean = mpz_cmp(lhs->digit, rhs->digit) > 0;
        break;
    case BINARY_LTE:
        value->boolean = mpz_cmp(lhs->digit, rhs->digit) <= 0;
        break;
    case BINARY_GTE:
        value->boolean = mpz_cmp(lhs->digit, rhs->digit) >= 0;
        break;

    default:
        /* TODO: how does gmp do bitshifts */
        assert("unknown binary op %d", binary);
        break;
    }
    
    sanitize_value(state, value);

    return value;
}

static value_t *eval_unary(state_t *state, type_t *type, unary_t unary, operand_t val) {
    value_t *value = build_value(type);
    value_t *expr = make_value(state, val);

    switch (unary) {
    case UNARY_NEG:
        mpz_neg(value->digit, expr->digit);
        break;
    case UNARY_ABS:
        mpz_abs(value->digit, expr->digit);
        break;

    case UNARY_REF:
        value->values[0] = expr;
        break;
    case UNARY_DEREF:
        value = expr->values[0];
        break;

    default:
        assert("unknown unary op %d", unary);
        break;
    }

    sanitize_value(state, value);

    return value;
}

static value_t *eval_convert(state_t *state, type_t *type, operand_t val) {
    value_t *value = build_value(type);
    value_t *other = make_value(state, val);

    if (is_boolean(type)) {
        if (is_boolean(other->type)) {
            value->boolean = other->boolean;
        } else if (is_integer(other->type)) {
            value->boolean = mpz_cmp_si(other->digit, 0) != 0;
        } else {
            value->boolean = false;
            reportid_t id = step_report(LEVEL_ERROR, state, "cannot convert to %s at compile time", typefmt(type));
            report_underline(id, typefmt(other->type));
        }
        return value;
    }

    if (is_integer(type)) {
        if (is_integer(other->type)) {
            mpz_set(value->digit, other->digit);
            if (!is_signed(type) && mpz_sgn(value->digit) < 0) {
                step_report(LEVEL_WARNING, state, "converting negative signed int to unsigned int, truncating to 0");
                mpz_set_ui(value->digit, 0);
            }
        } else if (is_pointer(other->type)) {
            if (get_integer_kind(type) != INTEGER_INTPTR) {
                reportid_t id = step_report(LEVEL_ERROR, state, "cannot convert to %s at compile time", typefmt(type));
                report_underline(id, typefmt(other->type));
                report_note(id, "consider casting to (u)intptr");
            }
            mpz_set_ui(value->digit, (uintptr_t)other->values);
        }

        return value;
    }

    value_t *out = empty_value();
    report_uninit(state, out);
    return out;
}

/* evaluate a step, return false if a value has been returned */
static bool eval_step(state_t *state) {
    /* prevent out of bounds access */
    if (state->ip > state->flow->len) {
        return false;
    }

    step_t *step = step_at(state->flow, state->ip);

    switch (step->opcode) {
    case OP_VALUE:
        state->values[state->ip] = make_value(state, step->value);
        return true;
    case OP_BINARY:
        state->values[state->ip] = eval_binary(state, step->type, step->binary, step->lhs, step->rhs);
        return true;
    case OP_UNARY:
        state->values[state->ip] = eval_unary(state, step->type, step->unary, step->expr);
        return true;
    case OP_BRANCH:
        state->ip = get_cond(state, step->cond)
            ? get_dst(step->block)
            : get_dst(step->other);
        return true;
    case OP_JUMP:
        state->ip = get_dst(step->block);
        return true;
    case OP_LOAD:
        state->values[state->ip] = make_value(state, step->src);
        return true;
    case OP_RETURN:
        state->result = make_value(state, step->value); 
        return false;
    case OP_CONVERT:
        state->values[state->ip] = eval_convert(state, step->type, step->value);
        return true;

    case OP_BLOCK: case OP_EMPTY: 
        return true;
    default:
        assert("[%s @ %zu] unknown step eval %d", flow_name(state->flow), state->ip, step->opcode);
        return false;
    }
}

static void fill_values(value_t **values, size_t len) {
    for (size_t i = 0; i < len; i++) {
        values[i] = empty_value();
    }
}

static state_t new_state(module_t *mod, flow_t *flow) {
    state_t state;

    state.mod = mod;
    state.flow = flow;
    state.ip = 0;

    state.values = ctu_malloc(sizeof(value_t *) * flow->len);
    state.args = ctu_malloc(sizeof(value_t *) * flow->nargs);
    state.result = empty_value();

    state.error = false;

    fill_values(state.values, flow->len);
    fill_values(state.args, flow->nargs);

    return state;
}

value_t *eval_global(module_t *mod, flow_t *flow) {
    if (flow->value) {
        return flow->value;
    }

    state_t state = new_state(mod, flow);

    while (eval_step(&state)) {
        
        if (state.error) {
            return empty_value();
        }

        state.ip++;
    }

    /* TODO: free values properly */
    ctu_free(state.values);
    ctu_free(state.args);

    return state.result;
}
