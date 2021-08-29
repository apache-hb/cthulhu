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

lir_t *lir_declare(node_t *node, const char *name, leaf_t expected) {
    lir_t *lir = lir_decl(node, LIR_EMPTY, name);
    lir->expected = expected;
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
    if (dst->leaf != LIR_EMPTY || dst->expected != LIR_VALUE) {
        assert("lir-value already resolved");
    }

    dst->leaf = LIR_VALUE;
    dst->type = type;
    dst->init = init;
}

void lir_resolve(lir_t *lir, type_t *type) {
    lir->type = type;
}

type_t *lir_resolved(lir_t *lir) {
    return lir->type;
}
