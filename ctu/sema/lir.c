#include "lir.h"

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
