#include "common.h"

#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/type.h"

hlir_t *hlir_error(node_t node, const char *error)
{
    hlir_t *self = hlir_new(node, kMetaType, HLIR_ERROR);
    self->error = error;
    return self;
}

hlir_t *hlir_digit_literal(node_t node, const hlir_t *type, mpz_t value)
{
    hlir_t *self = hlir_new(node, type, HLIR_DIGIT_LITERAL);
    mpz_init_set(self->digit, value);
    return self;
}

hlir_t *hlir_int_literal(node_t node, const hlir_t *type, int value)
{
    hlir_t *self = hlir_new(node, type, HLIR_DIGIT_LITERAL);
    mpz_init_set_si(self->digit, value);
    return self;
}

hlir_t *hlir_bool_literal(node_t node, const hlir_t *type, bool value)
{
    hlir_t *self = hlir_new(node, type, HLIR_BOOL_LITERAL);
    self->boolean = value;
    return self;
}

hlir_t *hlir_string_literal(node_t node, const hlir_t *type, const char *value, size_t length)
{
    hlir_t *self = hlir_new(node, type, HLIR_STRING_LITERAL);
    self->string = value;
    self->stringLength = length;
    return self;
}

hlir_t *hlir_name(node_t node, hlir_t *read)
{
    hlir_t *self = hlir_new(node, get_hlir_type(read), HLIR_NAME);
    self->read = read;
    return self;
}

hlir_t *hlir_unary(node_t node, const hlir_t *type, hlir_t *operand, unary_t unary)
{
    hlir_t *self = hlir_new(node, type, HLIR_UNARY);
    self->unary = unary;
    self->operand = operand;
    return self;
}

hlir_t *hlir_binary(node_t node, const hlir_t *type, binary_t binary, hlir_t *lhs, hlir_t *rhs)
{
    hlir_t *self = hlir_new(node, type, HLIR_BINARY);
    self->lhs = lhs;
    self->rhs = rhs;
    self->binary = binary;
    return self;
}

hlir_t *hlir_compare(node_t node, const hlir_t *type, compare_t compare, hlir_t *lhs, hlir_t *rhs)
{
    hlir_t *self = hlir_new(node, type, HLIR_COMPARE);
    self->lhs = lhs;
    self->rhs = rhs;
    self->compare = compare;
    return self;
}

hlir_t *hlir_call(node_t node, hlir_t *call, vector_t *args)
{
    hlir_t *self = hlir_new(node, NULL, HLIR_CALL);
    self->call = call;
    self->args = args;
    return self;
}

hlir_t *hlir_stmts(node_t node, vector_t *stmts)
{
    hlir_t *self = hlir_new(node, kInvalidNode, HLIR_STMTS);
    self->stmts = stmts;
    return self;
}

hlir_t *hlir_branch(node_t node, hlir_t *cond, hlir_t *then, hlir_t *other)
{
    hlir_t *self = hlir_new(node, kInvalidNode, HLIR_BRANCH);
    self->cond = cond;
    self->then = then;
    self->other = other;
    return self;
}

hlir_t *hlir_loop(node_t node, hlir_t *cond, hlir_t *body, hlir_t *other)
{
    hlir_t *self = hlir_new(node, kInvalidNode, HLIR_LOOP);
    self->cond = cond;
    self->then = body;
    self->other = other;
    return self;
}

hlir_t *hlir_assign(node_t node, hlir_t *dst, hlir_t *src)
{
    hlir_t *self = hlir_new(node, kInvalidNode, HLIR_ASSIGN);
    self->dst = dst;
    self->src = src;
    return self;
}

// building values

hlir_t *hlir_field(node_t node, const hlir_t *type, const char *name)
{
    return hlir_new_decl(node, name, type, HLIR_FIELD);
}

// building modules

void hlir_set_attributes(hlir_t *self, const hlir_attributes_t *attributes)
{
    self->attributes = attributes;
}

void hlir_set_parent(hlir_t *self, const hlir_t *parent)
{
    self->parentDecl = parent;
}
