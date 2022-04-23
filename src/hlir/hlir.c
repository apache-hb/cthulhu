#include "common.h"

#include "cthulhu/hlir/type.h"
#include "cthulhu/hlir/query.h"

hlir_t *hlir_error(const node_t *node, const char *error) {
    hlir_t *self = hlir_new(node, kMetaType, HLIR_ERROR);
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
    hlir_t *self = hlir_new(node, get_hlir_type(read), HLIR_NAME);
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
    hlir_t *self = hlir_new(node, kInvalidNode, HLIR_STMTS);
    self->stmts = stmts;
    return self;
}

hlir_t *hlir_branch(const node_t *node, hlir_t *cond, hlir_t *then, hlir_t *other) {
    hlir_t *self = hlir_new(node, kInvalidNode, HLIR_BRANCH);
    self->cond = cond;
    self->then = then;
    self->other = other;
    return self;
}

hlir_t *hlir_loop(const node_t *node, hlir_t *cond, hlir_t *body, hlir_t *other) {
    hlir_t *self = hlir_new(node, kInvalidNode, HLIR_LOOP);
    self->cond = cond;
    self->then = body;
    self->other = other;
    return self;
}

hlir_t *hlir_assign(const node_t *node, hlir_t *dst, hlir_t *src) {
    hlir_t *self = hlir_new(node, kInvalidNode, HLIR_ASSIGN);
    self->dst = dst;
    self->src = src;
    return self;
}

// building values

hlir_t *hlir_field(const node_t *node, const hlir_t *type, const char *name) {
    return hlir_new_decl(node, name, type, HLIR_FIELD);
}

// building modules

void hlir_set_attributes(hlir_t *self, const hlir_attributes_t *attributes) {
    self->attributes = attributes;
}
