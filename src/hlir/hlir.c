#include "cthulhu/hlir/hlir.h"

static hlir_t *hlir_new(node_t *node, hlir_type_t type) {
    hlir_t *hlir = ctu_malloc(sizeof(hlir_t));
    hlir->node = node;
    hlir->type = type;
    return hlir;
}

hlir_t *hlir_declare(node_t *node, const char *name, hlir_type_t expect) {
    hlir_t *hlir = hlir_new(node, HLIR_DECLARE);
    hlir->name = name;
    hlir->expect = expect;
    return hlir;
}

hlir_t *hlir_module(node_t *node, const char *mod, vector_t *imports, vector_t *globals, vector_t *defines) {
    hlir_t *hlir = hlir_new(node, HLIR_MODULE);
    hlir->mod = mod;
    hlir->imports = imports;
    hlir->globals = globals;
    hlir->defines = defines;
    return hlir;
}
