#include "common.h"

#include "base/panic.h"

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"

hlir_t *hlir_error(node_t *node, const char *error)
{
    hlir_t *self = hlir_new(node, kMetaType, eHlirError);
    self->name = error;
    self->of = self;
    return self;
}

hlir_t *hlir_digit_literal(node_t *node, const hlir_t *type, mpz_t value)
{
    hlir_t *self = hlir_new(node, type, eHlirDigitLiteral);
    mpz_init_set(self->digit, value);
    return self;
}

hlir_t *hlir_int_literal(node_t *node, const hlir_t *type, int value)
{
    hlir_t *self = hlir_new(node, type, eHlirDigitLiteral);
    mpz_init_set_si(self->digit, value);
    return self;
}

hlir_t *hlir_bool_literal(node_t *node, const hlir_t *type, bool value)
{
    hlir_t *self = hlir_new(node, type, eHlirBoolLiteral);
    self->boolean = value;
    return self;
}

hlir_t *hlir_string_literal(node_t *node, const hlir_t *type, struct string_view_t literal)
{
    hlir_t *self = hlir_new(node, type, eHlirStringLiteral);
    self->stringLiteral = literal;
    return self;
}

hlir_t *hlir_name(node_t *node, hlir_t *read)
{
    hlir_t *self = hlir_new(node, get_hlir_type(read), eHlirLoad);
    self->read = read;
    return self;
}

hlir_t *hlir_unary(node_t *node, struct hlir_unary_t expr)
{
    hlir_t *self = hlir_new(node, get_hlir_type(expr.operand), eHlirUnary);
    self->unaryExpr = expr;
    return self;
}

hlir_t *hlir_binary(node_t *node, const hlir_t *type, binary_t binary, hlir_t *lhs, hlir_t *rhs)
{
    hlir_t *self = hlir_new(node, type, eHlirBinary);
    self->lhs = lhs;
    self->rhs = rhs;
    self->binary = binary;
    return self;
}

hlir_t *hlir_compare(node_t *node, const hlir_t *type, compare_t compare, hlir_t *lhs, hlir_t *rhs)
{
    hlir_t *self = hlir_new(node, type, eHlirCompare);
    self->lhs = lhs;
    self->rhs = rhs;
    self->compare = compare;
    return self;
}

hlir_t *hlir_call(node_t *node, hlir_t *call, vector_t *args)
{
    CTASSERT(args != NULL);

    const hlir_t *signature = get_hlir_type(call);
    hlir_t *self = hlir_new(node, closure_result(signature), eHlirCall);
    self->call = call;
    self->args = args;
    return self;
}

hlir_t *hlir_stmts(node_t *node, vector_t *stmts)
{
    CTASSERT(stmts != NULL);

    hlir_t *self = hlir_new(node, kInvalidNode, eHlirStmts);
    self->stmts = stmts;
    return self;
}

hlir_t *hlir_branch(node_t *node, hlir_t *cond, hlir_t *then, hlir_t *other)
{
    hlir_t *self = hlir_new(node, kInvalidNode, eHlirBranch);
    self->cond = cond;
    self->then = then;
    self->other = other;
    return self;
}

hlir_t *hlir_loop(node_t *node, hlir_t *cond, hlir_t *body, hlir_t *other)
{
    hlir_t *self = hlir_new(node, kInvalidNode, eHlirLoop);
    self->cond = cond;
    self->then = body;
    self->other = other;
    return self;
}

hlir_t *hlir_break(node_t *node, hlir_t *target)
{
    hlir_t *self = hlir_new(node, kInvalidNode, eHlirBreak);
    self->target = target;
    return self;
}

hlir_t *hlir_continue(node_t *node, hlir_t *target)
{
    hlir_t *self = hlir_new(node, kInvalidNode, eHlirContinue);
    self->target = target;
    return self;
}

hlir_t *hlir_assign(node_t *node, hlir_t *dst, hlir_t *src)
{
    hlir_t *self = hlir_new(node, kInvalidNode, eHlirAssign);
    self->dst = dst;
    self->src = src;
    return self;
}

hlir_t *hlir_return(node_t *node, hlir_t *result)
{
    hlir_t *self = hlir_new(node, kInvalidNode, eHlirReturn);
    self->result = result;
    return self;
}

// building values

hlir_t *hlir_field(node_t *node, const hlir_t *type, const char *name)
{
    return hlir_new_decl(node, name, type, eHlirRecordField);
}

// building modules

void hlir_set_attributes(hlir_t *self, const hlir_attributes_t *attributes)
{
    self->attributes = attributes;
}
