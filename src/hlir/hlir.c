#include "cthulhu/hlir/hlir.h"

static const type_t FAILURE = { .type = TYPE_ERROR, .name = "INTERNAL FAILURE", NULL };

static hlir_t *hlir_new(const node_t *node, const type_t *of, hlir_type_t type) {
    hlir_t *self = ctu_malloc(sizeof(hlir_t));
    self->type = type;
    self->node = node;
    self->of = of;
    return self;
}

static hlir_t *hlir_new_decl(const node_t *node, const char *name, const type_t *of, hlir_type_t type) {
    hlir_t *hlir = hlir_new(node, of, type);
    hlir->name = name;
    return hlir;
}

const type_t *typeof_node(const hlir_t *hlir) {
    return hlir->of;
}

// building expressions

hlir_t *hlir_error(const node_t *node, const char *error) {
    hlir_t *self = hlir_new(node, &FAILURE, HLIR_ERROR);
    self->error = error;
    return self;
}

hlir_t *hlir_literal(const node_t *node, value_t *value) {
    hlir_t *self = hlir_new(node, typeof_value(value), HLIR_LITERAL);
    self->literal = value;
    return self;
}

hlir_t *hlir_name(const node_t *node, hlir_t *read) {
    hlir_t *self = hlir_new(node, typeof_node(read), HLIR_NAME);
    self->read = read;
    return self;
}

hlir_t *hlir_binary(const node_t *node, const type_t *type, binary_t binary, hlir_t *lhs, hlir_t *rhs) {
    hlir_t *self = hlir_new(node, type, HLIR_BINARY);
    self->lhs = lhs;
    self->rhs = rhs;
    self->binary = binary;
    return self;
}

hlir_t *hlir_compare(const node_t *node, const type_t *type, compare_t compare, hlir_t *lhs, hlir_t *rhs) {
    hlir_t *self = hlir_new(node, type, HLIR_COMPARE);
    self->lhs = lhs;
    self->rhs = rhs;
    self->compare = compare;
    return self;
}

hlir_t *hlir_call(const node_t *node, hlir_t *call, vector_t *args) {
    hlir_t *self = hlir_new(node, &FAILURE, HLIR_CALL);
    self->call = call;
    self->args = args;
    return self;
}

hlir_t *hlir_stmts(const node_t *node, vector_t *stmts) {
    hlir_t *self = hlir_new(node, &FAILURE, HLIR_STMTS);
    self->stmts = stmts;
    return self;
}

hlir_t *hlir_branch(const node_t *node, hlir_t *cond, hlir_t *then, hlir_t *other) {
    hlir_t *self = hlir_new(node, &FAILURE, HLIR_BRANCH);
    self->cond = cond;
    self->then = then;
    self->other = other;
    return self;
}

hlir_t *hlir_loop(const node_t *node, hlir_t *cond, hlir_t *body, hlir_t *other) {
    hlir_t *self = hlir_new(node, &FAILURE, HLIR_LOOP);
    self->cond = cond;
    self->then = body;
    self->other = other;
    return self;
}

hlir_t *hlir_assign(const node_t *node, hlir_t *dst, hlir_t *src) {
    hlir_t *self = hlir_new(node, &FAILURE, HLIR_ASSIGN);
    self->dst = dst;
    self->src = src;
    return self;
}

// building functions

hlir_t *hlir_new_function(const node_t *node, const char *name, const type_t *type) {
    hlir_t *hlir = hlir_new_decl(node, name, type, HLIR_FUNCTION);
    hlir->locals = vector_new(0);
    return hlir;
}

void hlir_add_local(hlir_t *self, hlir_t *local) {
    vector_push(&self->locals, local);
}

void hlir_build_function(hlir_t *self, hlir_t *body) {
    self->body = body;
}

// building values

hlir_t *hlir_new_value(const node_t *node, const char *name, const type_t *type) {
    return hlir_new_decl(node, name, type, HLIR_VALUE);
}

void hlir_build_value(hlir_t *self, hlir_t *value) {
    self->value = value;
}

hlir_t *hlir_value(const node_t *node, const char *name, const type_t *type, hlir_t *value) {
    hlir_t *self = hlir_new_value(node, name, type);
    hlir_build_value(self, value);
    return self;
}

// building modules

hlir_t *hlir_new_module(const node_t *node, const char *name) {
    return hlir_new_decl(node, name, &FAILURE, HLIR_MODULE);
}

void hlir_build_module(hlir_t *self, vector_t *imports, vector_t *values, vector_t *functions, vector_t *types) {
    self->imports = imports;
    self->globals = values;
    self->defines = functions;
    self->types = types;
}

hlir_t *hlir_import_function(const node_t *node, const char *name, const type_t *type) {
    return hlir_new_decl(node, name, type, HLIR_IMPORT_FUNCTION);
}

hlir_t *hlir_import_value(const node_t *node, const char *name, const type_t *type) {
    return hlir_new_decl(node, name, type, HLIR_IMPORT_VALUE);
}
