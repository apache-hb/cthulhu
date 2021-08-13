#include "eval.h"

#include "ctu/util/report.h"
#include "ctu/util/str.h"

typedef struct {
    module_t *mod;

    flow_t *flow;
    size_t ip;

    value_t **values;

    value_t **args;

    value_t *result;

    bool error;
} state_t;

static value_t *build_value(type_t *type, bool init) {
    value_t *value = ctu_malloc(sizeof(value_t));

    value->type = type;
    value->init = init;

    if (type && is_integer(type)) {
        mpz_init(value->digit);
    }

    return value;
}

static value_t *build_array(type_t *type, size_t len) {
    value_t *value = build_value(type, true);

    value->type = type;

    value->size = len;
    value->values = ctu_malloc(sizeof(value_t*) * len);
    
    for (size_t i = 0; i < len; i++) {
        value->values[i] = build_value(type, false);
    }

    return value;
}

value_t *empty_value(void) {
    return build_value(NULL, false);
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
    return !value->init;
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
    value_t *value = build_value(imm.type, true);

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

    value_t *value = build_value(type, true);
    value->func = flow;

    return value;
}

static value_t *string_value(module_t *mod, size_t idx) {
    value_t *value = build_value(STRING_TYPE, true);
    value->string = mod->strings[idx];
    return value;
}

static value_t *make_value(state_t *state, operand_t op) {
    if (op.offset != SIZE_MAX) {
        assert("field constinit not yet supported");
        return empty_value();
    }

    value_t *value;
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

    case STRING:
        value = string_value(state->mod, op.var);
        break;

    case NONE:
        assert("none in make-value");
        value = empty_value();
        break;

    default:
        value = empty_value();
        assert("make-value %d", op.kind);
        break;
    }

    if (report_uninit(state, value)) {

    }

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
    value_t *value = build_value(type, true);
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
        } else {
            mpz_div(value->digit, lhs->digit, rhs->digit);
        }
        break;
    case BINARY_REM:
        if (mpz_sgn(rhs->digit) == 0) {
            step_report(LEVEL_ERROR, state, "remainder by zero");
        } else {
            mpz_mod(value->digit, lhs->digit, rhs->digit);
        }
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

    case BINARY_SHL:
        mpz_mul_2exp(value->digit, lhs->digit, mpz_size(rhs->digit));
        break;
    case BINARY_SHR:
        mpz_div_2exp(value->digit, lhs->digit, mpz_size(rhs->digit));
        break;

    default:
        assert("unknown binary op %d", binary);
        break;
    }
    
    sanitize_value(state, value);

    return value;
}

static value_t *eval_unary(state_t *state, type_t *type, unary_t unary, operand_t val) {
    value_t *value = build_value(type, true);
    value_t *expr = make_value(state, val);

    switch (unary) {
    case UNARY_NEG:
        mpz_neg(value->digit, expr->digit);
        break;
    case UNARY_ABS:
        mpz_abs(value->digit, expr->digit);
        break;

    case UNARY_REF:
        value->value = expr;
        break;
    case UNARY_DEREF:
        value = expr->value;
        break;

    default:
        assert("unknown unary op %d", unary);
        break;
    }

    sanitize_value(state, value);

    return value;
}

static value_t *eval_convert(state_t *state, type_t *type, operand_t val) {
    value_t *value = build_value(type, true);
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
        } else {
            assert(format("invalid integer cast from %s to %s", typefmt(type), typefmt(other->type)));
        }

        return value;
    }

    assert("unknown type cast");

    value_t *out = empty_value();
    report_uninit(state, out);
    return out;
}

static value_t *reserve(step_t *step) {
    size_t size = step->size;
    type_t *of = step->type;

    value_t *val = build_array(of, size);

    return val;
}

static value_t *get_offset(state_t *state, step_t *step) {
    ASSERT(step != NULL)("step was null");
    ASSERT(step->type != NULL)("step type was null");

    value_t *array = make_value(state, step->src);
    type_t *type = array->type;

    if (!is_array(type) && !is_pointer(type)) {
        assert("cannot get offset of non-array type");
        return empty_value();
    }

    value_t *offset = make_value(state, step->index);
    if (!is_integer(offset->type)) {
        assert("cannot get offset from non-integer type");
        return empty_value();
    }

    size_t shift = mpz_get_ui(offset->digit);

    if (shift >= array->size) {
        reportid_t id = step_report(LEVEL_ERROR, state, "array index `%zu` out of bounds", shift);
        report_underline(id, format("original array size is %zu", array->size));

        return empty_value();
    }

    value_t *out = build_value(type, true);
    out->size = array->size - shift;
    out->values = array->values + shift;

    return out;
}

static void do_store(state_t *state, step_t *step) {
    value_t *dst = make_value(state, step->dst);
    value_t *src = make_value(state, step->src);

    dst->value = src;
}

/* evaluate a step, return false if a value has been returned */
static bool eval_step(state_t *state) {
    size_t ip = state->ip;

    /* prevent out of bounds access */
    if (ip > state->flow->len) {
        return false;
    }

    step_t *step = step_at(state->flow, ip);
    opcode_t op = step->opcode;

    printf("ip: %zu\n", ip);

    switch (op) {
    case OP_VALUE:
        state->values[ip] = make_value(state, step->value);
        return true;
    case OP_BINARY:
        state->values[ip] = eval_binary(state, step->type, step->binary, step->lhs, step->rhs);
        return true;
    case OP_UNARY:
        state->values[ip] = eval_unary(state, step->type, step->unary, step->expr);
        return true;
    case OP_BRANCH:
        ip = get_cond(state, step->cond)
            ? get_dst(step->block)
            : get_dst(step->other);
        return true;
    case OP_JUMP:
        ip = get_dst(step->block);
        return true;
    case OP_LOAD:
        state->values[ip] = make_value(state, step->src);
        return true;
    case OP_RETURN:
        state->result = make_value(state, step->value); 
        return false;
    case OP_CONVERT:
        state->values[ip] = eval_convert(state, step->type, step->value);
        return true;

    case OP_RESERVE:
        state->values[ip] = reserve(step);
        return true;

    case OP_OFFSET:
        state->values[ip] = get_offset(state, step);
        return true;

    case OP_STORE:
        do_store(state, step);
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

    value_t *result = state.result;
    result->init = true;

    return result;
}
