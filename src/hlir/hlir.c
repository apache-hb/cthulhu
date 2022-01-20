#include "cthulhu/hlir/hlir.h"

static hlir_t *hlir_new(const node_t *node, hlir_type_t kind, const type_t *type) {
    hlir_t *hlir = ctu_malloc(sizeof(hlir_t));
    hlir->node = node;
    hlir->kind = kind;
    hlir->type = type;
    return hlir;
}

hlir_t *hlir_digit(const node_t *node, const type_t *type, mpz_t digit) {
    hlir_t *hlir = hlir_new(node, HLIR_DIGIT, type);
    mpz_init_set(hlir->digit, digit);
    return hlir;
}

hlir_t *hlir_int(const node_t *node, const type_t *type, uintmax_t digit) {
    hlir_t *hlir = hlir_new(node, HLIR_DIGIT, type);
    mpz_init_set_ui(hlir->digit, digit);
    return hlir;
}

hlir_t *hlir_string(const node_t *node, const type_t *type, const char *str) {
    hlir_t *hlir = hlir_new(node, HLIR_STRING, type);
    hlir->string = str;
    return hlir;
}

hlir_t *hlir_name(const node_t *node, const type_t *type, hlir_t *hlir) {
    hlir_t *name = hlir_new(node, HLIR_NAME, type);
    name->ident = hlir;
    return name;
}

hlir_t *hlir_binary(const node_t *node, const type_t *type, hlir_t *lhs, hlir_t *rhs, binary_t op) {
    hlir_t *hlir = hlir_new(node, HLIR_BINARY, type);
    hlir->lhs = lhs;
    hlir->rhs = rhs;
    hlir->binary = op;
    return hlir;
}

hlir_t *hlir_call(const node_t *node, const type_t *type, hlir_t *function, vector_t *args) {
    hlir_t *hlir = hlir_new(node, HLIR_CALL, type);
    hlir->call = function;
    hlir->args = args;
    return hlir;
}

hlir_t *hlir_assign(const node_t *node, hlir_t *dst, hlir_t *src) {
    hlir_t *hlir = hlir_new(node, HLIR_ASSIGN, NULL);
    hlir->lhs = dst;
    hlir->rhs = src;
    return hlir;
}

hlir_t *hlir_value(const node_t *node, const type_t *type, const char *name, hlir_t *value) {
    hlir_t *hlir = hlir_new(node, HLIR_VALUE, type);
    hlir->name = name;
    hlir->value = value;
    return hlir;
}

hlir_t *hlir_function(const node_t *node, const type_t *type, const char *name, vector_t *body) {
    hlir_t *hlir = hlir_new(node, HLIR_FUNCTION, type);
    hlir->name = name;
    hlir->body = body;
    return hlir;
}

hlir_t *hlir_declare(const node_t *node, const char *name, hlir_type_t expect) {
    hlir_t *hlir = hlir_new(node, HLIR_DECLARE, NULL);
    hlir->name = name;
    hlir->expect = expect;
    return hlir;
}

hlir_t *hlir_module(const node_t *node, const char *mod, vector_t *imports, vector_t *globals, vector_t *defines) {
    hlir_t *hlir = hlir_new(node, HLIR_MODULE, NULL);
    hlir->mod = mod;
    hlir->imports = imports;
    hlir->globals = globals;
    hlir->defines = defines;
    return hlir;
}

hlir_t *hlir_error(const node_t *node, const type_t *type) {
    hlir_t *hlir = hlir_new(node, HLIR_ERROR, type);
    return hlir;
}
