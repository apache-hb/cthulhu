#include "lir.h"

#include "ctu/util/report.h"

static lir_t *lir_new(node_t *node, const type_t *type, leaf_t leaf) {
    lir_t *lir = ctu_malloc(sizeof(lir_t));
    lir->node = node;
    lir->leaf = leaf;
    lir->type = type;
    lir->data = NULL;
    return lir;
}

static const attrib_t DEFAULT_ATTRIBS = {
    .visibility = PRIVATE,
    .mangle = NULL
};

static lir_t *lir_decl(node_t *node, leaf_t leaf, const char *name) {
    lir_t *lir = lir_new(node, type_poison("forward declaration"), leaf);
    lir->name = name;
    lir->attribs = &DEFAULT_ATTRIBS;
    return lir;
}

void lir_attribs(lir_t *dst, const attrib_t *attribs) {
    dst->attribs = attribs;
}

lir_t *lir_forward(node_t *node, const char *name, leaf_t expected, void *ctx) {
    lir_t *lir = lir_decl(node, LIR_FORWARD, name);
    lir->expected = expected;
    lir->ctx = ctx;
    return lir;
}

lir_t *lir_module(node_t *node, 
                  vector_t *imports, 
                  vector_t *vars, 
                  vector_t *funcs) {
    lir_t *lir = lir_new(node, NULL, LIR_MODULE);

    lir->imports = imports;
    lir->vars = vars;
    lir->funcs = funcs;

    return lir;
}

lir_t *lir_int(node_t *node, const type_t *type, int digit) {
    lir_t *lir = lir_new(node, type, LIR_DIGIT);

    mpz_init_set_si(lir->digit, digit);

    return lir;
}

lir_t *lir_digit(node_t *node, const type_t *type, mpz_t digit) {
    lir_t *lir = lir_new(node, type, LIR_DIGIT);

    mpz_init_set(lir->digit, digit);

    return lir;
}

lir_t *lir_string(node_t *node, const type_t *type, const char *str) {
    lir_t *lir = lir_new(node, type, LIR_STRING);

    lir->str = str;

    return lir;
}

lir_t *lir_bool(node_t *node, const type_t *type, bool value) {
    lir_t *lir = lir_new(node, type, LIR_BOOL);

    lir->boolean = value;

    return lir;
}

lir_t *lir_access(node_t *node, const type_t *type, lir_t *it, size_t field) {
    lir_t *lir = lir_new(node, type, LIR_ACCESS);

    lir->it = it;
    lir->offset = field;

    return lir;
}

lir_t *lir_name(node_t *node, const type_t *type, lir_t *it) {
    lir_t *lir = lir_new(node, type, LIR_NAME);

    lir->it = it;

    return lir;
}

lir_t *lir_binary(node_t *node, const type_t *type, binary_t binary, lir_t *lhs, lir_t *rhs) {
    lir_t *lir = lir_new(node, type, LIR_BINARY);

    lir->binary = binary;
    lir->lhs = lhs;
    lir->rhs = rhs;

    return lir;
}

lir_t *lir_unary(node_t *node, const type_t *type, unary_t unary, lir_t *operand) {
    lir_t *lir = lir_new(node, type, LIR_UNARY);

    lir->unary = unary;
    lir->operand = operand;

    return lir;
}

lir_t *lir_call(node_t *node, const type_t *type, lir_t *func, vector_t *args) {
    lir_t *lir = lir_new(node, type, LIR_CALL);

    lir->func = func;
    lir->args = args;

    return lir;
}

lir_t *lir_assign(node_t *node, lir_t *dst, lir_t *src) {
    lir_t *lir = lir_new(node, NULL, LIR_ASSIGN);

    lir->dst = dst;
    lir->src = src;

    return lir;
}

lir_t *lir_while(node_t *node, lir_t *cond, lir_t *then) {
    lir_t *lir = lir_new(node, NULL, LIR_WHILE);

    lir->cond = cond;
    lir->then = then;

    return lir;
}

lir_t *lir_return(node_t *node, lir_t *operand) {
    lir_t *lir = lir_new(node, NULL, LIR_RETURN);
    lir->operand = operand;
    return lir;
}

lir_t *lir_stmts(node_t *node, vector_t *stmts) {
    lir_t *lir = lir_new(node, NULL, LIR_STMTS);

    lir->stmts = stmts;

    return lir;
}

lir_t *lir_branch(node_t *node, lir_t *cond, lir_t *then, lir_t *other) {
    lir_t *lir = lir_new(node, NULL, LIR_BRANCH);

    lir->cond = cond;
    lir->then = then;
    lir->other = other;

    return lir;
}

void lir_value(reports_t *reports, lir_t *dst, const type_t *type, lir_t *init) {
    if (dst->leaf != LIR_FORWARD) {
        ctu_assert(reports, "lir-value already resolved");
    }

    dst->leaf = LIR_VALUE;
    dst->type = type;
    dst->init = init;
}

void lir_define(reports_t *reports, lir_t *dst, const type_t *type, vector_t *locals, lir_t *body) {
    if (dst->leaf != LIR_FORWARD) {
        ctu_assert(reports, "lir-define already resolved");
    }

    dst->leaf = LIR_DEFINE;
    dst->type = type;
    dst->locals = locals;
    dst->body = body;   
}

lir_t *lir_param(node_t *node, const char *name, const type_t *type, size_t index) {
    lir_t *lir = lir_decl(node, LIR_PARAM, name);
    lir->type = type;
    lir->index = index;
    return lir;
}

lir_t *lir_poison(node_t *node, const char *msg) {
    return lir_new(node, type_poison(msg), LIR_POISON);
}

bool lir_ok(const lir_t *lir) {
    return !lir_is(lir, LIR_POISON);
}

bool lir_is(const lir_t *lir, leaf_t leaf) {
    return lir->leaf == leaf;
}

vector_t *lir_recurses(lir_t *lir, const lir_t *root) {
    vector_t *result = NULL;
    lir_t *source = NULL;

    if (lir == root) {
        return vector_init(lir);
    }

    switch (lir->leaf) {
    case LIR_NAME:
        result = lir_recurses(lir->it, root);
        source = lir->it;
        break;

    case LIR_VALUE:
        result = lir_recurses(lir->init, root);
        source = lir->init;
        break;

    case LIR_BINARY:
        if ((result = lir_recurses(lir->lhs, root))) {
            source = lir->lhs;
            break;
        }
        
        if ((result = lir_recurses(lir->rhs, root))) {
            source = lir->rhs;
            break;
        }
        break;

    case LIR_UNARY:
        result = lir_recurses(lir->operand, root);
        source = lir->operand;
        break;

    default:
        return NULL;
    }

    if (result) {
        vector_push(&result, source);
    }

    return result;
}

const type_t *lir_type(const lir_t *lir) {
    return lir->type;
}
