#include "lir.h"

static lir_t *lir_new(node_t *node, leaf_t leaf) {
    lir_t *lir = ctu_malloc(sizeof(lir_t));
    lir->leaf = leaf;
    lir->node = node;
    return lir;
}

lir_t *lir_module(node_t *node, vector_t *vars, vector_t *funcs) {
    lir_t *lir = lir_new(node, LIR_MODULE);

    lir->vars = vars;
    lir->funcs = funcs;

    return lir;
}
