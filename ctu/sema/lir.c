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

lir_t *lir_declare(node_t *node, const char *name, leaf_t expected, struct sema_t *sema) {
    lir_t *lir = lir_decl(node, LIR_EMPTY, name);
    lir->expected = expected;
    lir->sema = sema;
    return lir;
}

lir_t *lir_module(node_t *node, vector_t *vars, vector_t *funcs) {
    lir_t *lir = lir_new(node, LIR_MODULE);

    lir->vars = vars;
    lir->funcs = funcs;

    return lir;
}

lir_t *lir_digit(node_t *node, mpz_t digit) {
    lir_t *lir = lir_new(node, LIR_DIGIT);

    mpz_init_set(lir->digit, digit);

    return lir;
}

void lir_value(lir_t *dst, type_t *type, lir_t *init) {
    if (dst->leaf != LIR_VALUE) {
        assert("lir-value already resolved");
    }

    dst->type = type;
    dst->init = init;
}

void lir_begin(lir_t *dst, leaf_t leaf) {
    if (dst->leaf != LIR_EMPTY) {
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

bool lir_ok(lir_t *lir) {
    return !lir_is(lir, LIR_POISON);
}

bool lir_is(lir_t *lir, leaf_t leaf) {
    return lir->leaf == leaf;
}

void lir_resolve(lir_t *lir, type_t *type) {
    lir->type = type;
}

type_t *lir_resolved(lir_t *lir) {
    if (lir->leaf != LIR_EMPTY) {
        return lir->type;
    } else {
        return NULL;
    }
}
