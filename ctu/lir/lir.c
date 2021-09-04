#include "lir.h"

#include "ctu/util/report.h"

static lir_t *lir_new(node_t *node, leaf_t leaf) {
    lir_t *lir = ctu_malloc(sizeof(lir_t));
    lir->node = node;
    lir->leaf = leaf;
    lir->type = NULL;
    return lir;
}

static lir_t *lir_decl(node_t *node, leaf_t leaf, const char *name) {
    lir_t *lir = lir_new(node, leaf);
    lir->name = name;
    return lir;
}

lir_t *lir_forward(node_t *node, const char *name, leaf_t expected, void *ctx) {
    lir_t *lir = lir_decl(node, LIR_FORWARD, name);
    lir->expected = expected;
    lir->ctx = ctx;
    return lir;
}

lir_t *lir_module(node_t *node, vector_t *vars, vector_t *funcs) {
    lir_t *lir = lir_new(node, LIR_MODULE);

    lir->vars = vars;
    lir->funcs = funcs;

    return lir;
}

lir_t *lir_int(node_t *node, int digit) {
    lir_t *lir = lir_new(node, LIR_DIGIT);

    mpz_init_set_si(lir->digit, digit);

    return lir;
}

lir_t *lir_digit(node_t *node, mpz_t digit) {
    lir_t *lir = lir_new(node, LIR_DIGIT);

    mpz_init_set(lir->digit, digit);

    return lir;
}

lir_t *lir_binary(node_t *node, binary_t binary, lir_t *lhs, lir_t *rhs) {
    lir_t *lir = lir_new(node, LIR_BINARY);

    lir->binary = binary;
    lir->lhs = lhs;
    lir->rhs = rhs;

    return lir;
}

lir_t *lir_unary(node_t *node, unary_t unary, lir_t *operand) {
    lir_t *lir = lir_new(node, LIR_UNARY);

    lir->unary = unary;
    lir->operand = operand;

    return lir;
}

lir_t *lir_call(node_t *node, lir_t *func, vector_t *args) {
    lir_t *lir = lir_new(node, LIR_CALL);

    lir->func = func;
    lir->args = args;

    return lir;
}

lir_t *lir_assign(node_t *node, lir_t *dst, lir_t *src) {
    lir_t *lir = lir_new(node, LIR_ASSIGN);

    lir->dst = dst;
    lir->src = src;

    return lir;
}

lir_t *lir_while(node_t *node, lir_t *cond, lir_t *then) {
    lir_t *lir = lir_new(node, LIR_WHILE);

    lir->cond = cond;
    lir->then = then;

    return lir;
}

lir_t *lir_stmts(node_t *node, vector_t *stmts) {
    lir_t *lir = lir_new(node, LIR_STMTS);

    lir->stmts = stmts;

    return lir;
}

lir_t *lir_branch(node_t *node, lir_t *cond, lir_t *then, lir_t *other) {
    lir_t *lir = lir_new(node, LIR_BRANCH);

    lir->cond = cond;
    lir->then = then;
    lir->other = other;

    return lir;
}

void lir_value(lir_t *dst, type_t *type, lir_t *init) {
    if (dst->leaf != LIR_FORWARD) {
        assert("lir-value already resolved");
    }

    dst->leaf = LIR_VALUE;
    dst->type = type;
    dst->init = init;
}

void lir_define(lir_t *dst, type_t *type, vector_t *locals, lir_t *body) {
    if (dst->leaf != LIR_FORWARD) {
        assert("lir-define already resolved");
    }

    dst->leaf = LIR_DEFINE;
    dst->type = type;
    dst->locals = locals;
    dst->body = body;   
}

void lir_begin(lir_t *dst, leaf_t leaf) {
    if (dst->leaf != LIR_FORWARD) {
        assert("lir-begin already began");
        return;
    }

    if (dst->expected != leaf) {
        assert("lir-begin unexpected leaf");
        return;
    }

    dst->leaf = leaf;
}

lir_t *lir_poison(node_t *node, const char *msg) {
    lir_t *lir = lir_new(node, LIR_POISON);

    lir->msg = msg;

    return lir;
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
        return vector_init(lir->node);
    }

    switch (lir->leaf) {
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
        vector_push(&result, source->node);
    }

    return result;
}
