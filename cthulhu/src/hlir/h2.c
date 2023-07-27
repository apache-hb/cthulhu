#include "common.h"

#include "base/memory.h"

h2_t *h2_new(h2_kind_t kind, const node_t *node, const h2_t *type)
{
    h2_t *self = ctu_malloc(sizeof(h2_t));

    self->kind = kind;
    self->node = node;
    self->type = type;

    return self;
}

h2_t *h2_decl(h2_kind_t kind, const node_t *node, const h2_t *type, const char *name)
{
    h2_t *self = h2_new(kind, node, type);

    self->name = name;

    return self;
}

h2_t *h2_error(const node_t *node, const char *message)
{
    h2_t *self = h2_new(eHlir2Error, node, NULL);

    self->message = message;

    return self;
}

///
/// types
///

h2_t *h2_type_empty(const node_t *node, const char *name)
{
    return h2_decl(eHlir2TypeUnit, node, NULL, name);
}

h2_t *h2_type_unit(const node_t *node, const char *name)
{
    return h2_decl(eHlir2TypeUnit, node, NULL, name);
}

h2_t *h2_type_bool(const node_t *node, const char *name)
{
    return h2_decl(eHlir2TypeBool, node, NULL, name);
}

h2_t *h2_type_digit(const node_t *node, const char *name, digit_t digit, sign_t sign)
{
    h2_t *self = h2_decl(eHlir2TypeDigit, node, NULL, name);
    self->digit = digit;
    self->sign = sign;
    return self;
}

h2_t *h2_type_string(const node_t *node, const char *name)
{
    return h2_decl(eHlir2TypeString, node, NULL, name);
}

h2_t *h2_type_closure(const node_t *node, const char *name, const h2_t *result, vector_t *params, arity_t arity)
{
    h2_t *self = h2_decl(eHlir2TypeClosure, node, NULL, name);
    self->result = result;
    self->params = params;
    self->arity = arity;
    return self;
}

h2_t *h2_qualify(const node_t *node, const h2_t *type, quals_t quals)
{
    h2_t *self = h2_new(eHlir2Qualify, node, type);
    self->quals = quals;
    self->qualify = type;
    return self;
}

///
/// literal expressions
///

h2_t *h2_expr_empty(const node_t *node, const h2_t *type)
{
    return h2_new(eHlir2ExprEmpty, node, type);
}

h2_t *h2_expr_unit(const node_t *node, const h2_t *type)
{
    return h2_new(eHlir2ExprUnit, node, type);
}

h2_t *h2_expr_bool(const node_t *node, const h2_t *type, bool value)
{
    h2_t *self = h2_new(eHlir2ExprBool, node, type);
    self->boolValue = value;
    return self;
}

h2_t *h2_expr_digit(const node_t *node, const h2_t *type, mpz_t value)
{
    h2_t *self = h2_new(eHlir2ExprDigit, node, type);
    mpz_init_set(self->digitValue, value);
    return self;
}

h2_t *h2_expr_string(const node_t *node, const h2_t *type, const char *value, size_t length)
{
    h2_t *self = h2_new(eHlir2ExprString, node, type);
    self->stringValue = value;
    self->stringLength = length;
    return self;
}

///
/// expressions
///

h2_t *h2_expr_load(const node_t *node, h2_t *expr)
{
    h2_t *self = h2_new(eHlir2ExprLoad, node, expr->type);
    self->load = expr;
    return self;
}

h2_t *h2_expr_unary(const node_t *node, unary_t unary, h2_t *expr)
{
    h2_t *self = h2_new(eHlir2ExprUnary, node, expr->type);
    self->unary = unary;
    self->operand = expr;
    return self;
}

h2_t *h2_expr_binary(const node_t *node, const h2_t *type, binary_t binary, h2_t *lhs, h2_t *rhs)
{
    h2_t *self = h2_new(eHlir2ExprBinary, node, type);
    self->binary = binary;
    self->lhs = lhs;
    self->rhs = rhs;
    return self;
}

h2_t *h2_expr_compare(const node_t *node, const h2_t *type, compare_t compare, h2_t *lhs, h2_t *rhs)
{
    h2_t *self = h2_new(eHlir2ExprCompare, node, type);
    self->compare = compare;
    self->lhs = lhs;
    self->rhs = rhs;
    return self;
}

h2_t *h2_expr_call(const node_t *node, const h2_t *callee, vector_t *args)
{
    h2_t *self = h2_new(eHlir2ExprCall, node, callee->result);
    self->callee = callee;
    self->args = args;
    return self;
}

///
/// statements
///

h2_t *h2_stmt_block(const node_t *node, vector_t *stmts)
{
    h2_t *self = h2_new(eHlir2StmtBlock, node, NULL);
    self->stmts = stmts;
    return self;
}

h2_t *h2_stmt_return(const node_t *node, const h2_t *value)
{
    h2_t *self = h2_new(eHlir2StmtReturn, node, value->type);
    self->value = value;
    return self;
}

h2_t *h2_stmt_assign(const node_t *node, h2_t *dst, h2_t *src)
{
    h2_t *self = h2_new(eHlir2StmtAssign, node, dst->type);
    self->dst = dst;
    self->src = src;
    return self;
}

h2_t *h2_stmt_loop(const node_t *node, h2_t *cond, h2_t *body, h2_t *other)
{
    h2_t *self = h2_new(eHlir2StmtLoop, node, NULL);
    self->cond = cond;
    self->then = body;
    self->other = other;
    return self;
}

h2_t *h2_stmt_branch(const node_t *node, h2_t *cond, h2_t *then, h2_t *other)
{
    h2_t *self = h2_new(eHlir2StmtBranch, node, NULL);
    self->cond = cond;
    self->then = then;
    self->other = other;
    return self;
}
