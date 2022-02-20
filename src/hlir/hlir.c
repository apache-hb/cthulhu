#include "common.h"

static hlir_t *hlir_new_forward(const node_t *node, const char *name, const hlir_t *of, hlir_type_t expect) {
    hlir_t *hlir = hlir_new(node, of, HLIR_FORWARD);
    hlir->name = name;
    hlir->expected = expect;
    hlir->attributes = &DEFAULT_ATTRIBS;
    return hlir;
}

hlir_t *hlir_error(const node_t *node, const char *error) {
    hlir_t *self = hlir_new(node, NULL, HLIR_ERROR);
    self->error = error;
    return self;
}

hlir_t *hlir_digit_literal(const node_t *node, const hlir_t *type, mpz_t value) {
    hlir_t *self = hlir_new(node, type, HLIR_DIGIT_LITERAL);
    mpz_init_set(self->digit, value);
    return self;
}

hlir_t *hlir_int_literal(const node_t *node, const hlir_t *type, int value) {
    hlir_t *self = hlir_new(node, type, HLIR_DIGIT_LITERAL);
    mpz_init_set_si(self->digit, value);
    return self;
}

hlir_t *hlir_bool_literal(const node_t *node, const hlir_t *type, bool value) {
    hlir_t *self = hlir_new(node, type, HLIR_BOOL_LITERAL);
    self->boolean = value;
    return self;
}

hlir_t *hlir_string_literal(const node_t *node, const hlir_t *type, const char *value) {
    hlir_t *self = hlir_new(node, type, HLIR_STRING_LITERAL);
    self->string = value;
    return self;
}

hlir_t *hlir_name(const node_t *node, hlir_t *read) {
    hlir_t *self = hlir_new(node, typeof_hlir(read), HLIR_NAME);
    self->read = read;
    return self;
}

hlir_t *hlir_unary(const node_t *node, const hlir_t *type, hlir_t *operand, unary_t unary) {
    hlir_t *self = hlir_new(node, type, HLIR_UNARY);
    self->unary = unary;
    self->operand = operand;
    return self;
}

hlir_t *hlir_binary(const node_t *node, const hlir_t *type, binary_t binary, hlir_t *lhs, hlir_t *rhs) {
    hlir_t *self = hlir_new(node, type, HLIR_BINARY);
    self->lhs = lhs;
    self->rhs = rhs;
    self->binary = binary;
    return self;
}

hlir_t *hlir_compare(const node_t *node, const hlir_t *type, compare_t compare, hlir_t *lhs, hlir_t *rhs) {
    hlir_t *self = hlir_new(node, type, HLIR_COMPARE);
    self->lhs = lhs;
    self->rhs = rhs;
    self->compare = compare;
    return self;
}

hlir_t *hlir_call(const node_t *node, hlir_t *call, vector_t *args) {
    hlir_t *self = hlir_new(node, NULL, HLIR_CALL);
    self->call = call;
    self->args = args;
    return self;
}

hlir_t *hlir_stmts(const node_t *node, vector_t *stmts) {
    hlir_t *self = hlir_new(node, NULL, HLIR_STMTS);
    self->stmts = stmts;
    return self;
}

hlir_t *hlir_branch(const node_t *node, hlir_t *cond, hlir_t *then, hlir_t *other) {
    hlir_t *self = hlir_new(node, NULL, HLIR_BRANCH);
    self->cond = cond;
    self->then = then;
    self->other = other;
    return self;
}

hlir_t *hlir_loop(const node_t *node, hlir_t *cond, hlir_t *body, hlir_t *other) {
    hlir_t *self = hlir_new(node, NULL, HLIR_LOOP);
    self->cond = cond;
    self->then = body;
    self->other = other;
    return self;
}

hlir_t *hlir_assign(const node_t *node, hlir_t *dst, hlir_t *src) {
    hlir_t *self = hlir_new(node, NULL, HLIR_ASSIGN);
    self->dst = dst;
    self->src = src;
    return self;
}

// building functions

hlir_t *hlir_new_function(const node_t *node, 
                          const char *name, 
                          vector_t *params,
                          hlir_t *result,
                          bool variadic) {
    
    hlir_t *type = hlir_closure(node, name, params, result, variadic);
    hlir_t *hlir = hlir_new_forward(node, name, type, HLIR_FUNCTION);
    hlir->locals = vector_new(0);
    hlir->body = NULL;
    return hlir;
}

void hlir_add_local(hlir_t *self, hlir_t *local) {
    vector_push(&self->locals, local);
}

void hlir_build_function(hlir_t *self, hlir_t *body) {
    self->type = HLIR_FUNCTION;
    self->body = body;
}

// building values

hlir_t *hlir_new_value(const node_t *node, const char *name, const hlir_t *type) {
    return hlir_new_forward(node, name, type, HLIR_VALUE);
}

void hlir_build_value(hlir_t *self, hlir_t *value) {
    self->type = HLIR_VALUE;
    self->value = value;
}

hlir_t *hlir_value(const node_t *node, const char *name, const hlir_t *type, hlir_t *value) {
    hlir_t *self = hlir_new_value(node, name, type);
    hlir_build_value(self, value);
    return self;
}

hlir_t *hlir_new_struct(const node_t *node, const char *name) {
    hlir_t *self = hlir_new_forward(node, name, NULL, HLIR_STRUCT);
    self->fields = vector_new(4);
    return self;
}

hlir_t *hlir_new_union(const node_t *node, const char *name) {
    hlir_t *self = hlir_new_forward(node, name, NULL, HLIR_UNION);
    self->fields = vector_new(4);
    return self;
}

void hlir_add_field(hlir_t *self, hlir_t *field) {
    vector_push(&self->fields, field);
}

void hlir_build_struct(hlir_t *hlir) {
    hlir->type = HLIR_STRUCT;
}

void hlir_build_union(hlir_t *hlir) {
    hlir->type = HLIR_UNION;
}

hlir_t *hlir_new_alias(const node_t *node, const char *name) {
    return hlir_new_forward(node, name, NULL, HLIR_ALIAS);
}

void hlir_build_alias(hlir_t *self, hlir_t *type) {
    self->type = HLIR_ALIAS;
    self->alias = type;
}


// building modules

hlir_t *hlir_new_module(const node_t *node, const char *name) {
    return hlir_new_decl(node, name, NULL, HLIR_MODULE);
}

void hlir_build_module(hlir_t *self, vector_t *values, vector_t *functions, vector_t *types) {
    self->globals = values;
    self->defines = functions;
    self->types = types;
}

void hlir_set_attributes(hlir_t *self, const hlir_attributes_t *attributes) {
    self->attributes = attributes;
}
