#include "cthulhu/hlir/hlir.h"

static hlir_t *hlir_new(node_t *node, hlir_type_t kind, const type_t *type) {
    hlir_t *hlir = ctu_malloc(sizeof(hlir_t));
    hlir->node = node;
    hlir->kind = kind;
    hlir->type = type;
    return hlir;
}

hlir_t *hlir_digit(node_t *node, const type_t *type, mpz_t digit) {
    hlir_t *hlir = hlir_new(node, HLIR_DIGIT, type);
    mpz_init_set(hlir->digit, digit);
    return hlir;
}

hlir_t *hlir_int(node_t *node, const type_t *type, uintmax_t digit) {
    hlir_t *hlir = hlir_new(node, HLIR_DIGIT, type);
    mpz_init_set_ui(hlir->digit, digit);
    return hlir;
}

hlir_t *hlir_value(node_t *node, const type_t *type, const char *name, hlir_t *value) {
    hlir_t *hlir = hlir_new(node, HLIR_VALUE, type);
    hlir->name = name;
    hlir->value = value;
    return hlir;
}

hlir_t *hlir_declare(node_t *node, const char *name, hlir_type_t expect) {
    hlir_t *hlir = hlir_new(node, HLIR_DECLARE, NULL);
    hlir->name = name;
    hlir->expect = expect;
    return hlir;
}

hlir_t *hlir_module(node_t *node, const char *mod, vector_t *imports, vector_t *globals, vector_t *defines) {
    hlir_t *hlir = hlir_new(node, HLIR_MODULE, NULL);
    hlir->mod = mod;
    hlir->imports = imports;
    hlir->globals = globals;
    hlir->defines = defines;
    return hlir;
}
