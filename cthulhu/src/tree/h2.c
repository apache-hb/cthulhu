#include "common.h"

#include "base/memory.h"
#include "base/panic.h"

static const attribs_t kDefaultAttrib = {
    .link = eLinkModule,
    .visibility = eVisiblePrivate
};

tree_t *tree_new(tree_kind_t kind, const node_t *node, const tree_t *type)
{
    tree_t *self = ctu_malloc(sizeof(tree_t));

    self->kind = kind;
    self->node = node;
    self->type = type;

    return self;
}

tree_t *tree_decl(tree_kind_t kind, const node_t *node, const tree_t *type, const char *name)
{
    CTASSERT(name != NULL);

    tree_t *self = tree_new(kind, node, type);

    self->name = name;
    self->attrib = &kDefaultAttrib;
    self->resolve = NULL;

    return self;
}

tree_t *tree_error(const node_t *node, const char *message)
{
    CTASSERT(message != NULL);

    tree_t *self = tree_new(eTreeError, node, NULL);

    self->type = self;
    self->message = message;

    return self;
}

///
/// types
///

tree_t *tree_type_empty(const node_t *node, const char *name)
{
    return tree_decl(eTreeTypeUnit, node, NULL, name);
}

tree_t *tree_type_unit(const node_t *node, const char *name)
{
    return tree_decl(eTreeTypeUnit, node, NULL, name);
}

tree_t *tree_type_bool(const node_t *node, const char *name)
{
    return tree_decl(eTreeTypeBool, node, NULL, name);
}

tree_t *tree_type_digit(const node_t *node, const char *name, digit_t digit, sign_t sign)
{
    tree_t *self = tree_decl(eTreeTypeDigit, node, NULL, name);
    self->digit = digit;
    self->sign = sign;
    return self;
}

tree_t *tree_type_string(const node_t *node, const char *name)
{
    return tree_decl(eTreeTypeString, node, NULL, name);
}

tree_t *tree_type_closure(const node_t *node, const char *name, const tree_t *result, vector_t *params, arity_t arity)
{
    CTASSERT(result != NULL);
    CTASSERT(params != NULL);

    tree_t *self = tree_decl(eTreeTypeClosure, node, NULL, name);
    self->result = result;
    self->params = params;
    self->arity = arity;
    return self;
}

tree_t *tree_type_pointer(const node_t *node, const char *name, tree_t *pointer)
{
    CTASSERT(pointer != NULL);

    tree_t *self = tree_decl(eTreeTypePointer, node, NULL, name);
    self->pointer = pointer;
    return self;
}

tree_t *tree_qualify(const node_t *node, const tree_t *type, quals_t quals)
{
    CTASSERT(type != NULL);

    tree_t *self = tree_new(eTreeQualify, node, type);
    self->quals = quals;
    self->qualify = type;
    return self;
}

///
/// literal expressions
///

tree_t *tree_expr_empty(const node_t *node, const tree_t *type)
{
    return tree_new(eTreeExprEmpty, node, type);
}

tree_t *tree_expr_unit(const node_t *node, const tree_t *type)
{
    return tree_new(eTreeExprUnit, node, type);
}

tree_t *tree_expr_bool(const node_t *node, const tree_t *type, bool value)
{
    tree_t *self = tree_new(eTreeExprBool, node, type);
    self->boolValue = value;
    return self;
}

tree_t *tree_expr_digit(const node_t *node, const tree_t *type, const mpz_t value)
{
    tree_t *self = tree_new(eTreeExprDigit, node, type);
    mpz_init_set(self->digitValue, value);
    return self;
}

tree_t *tree_expr_string(const node_t *node, const tree_t *type, const char *value, size_t length)
{
    CTASSERT(value != NULL);

    tree_t *self = tree_new(eTreeExprString, node, type);
    self->stringValue = value;
    self->stringLength = length;
    return self;
}

///
/// expressions
///

tree_t *tree_expr_load(const node_t *node, tree_t *expr)
{
    CTASSERT(expr != NULL);

    tree_t *self = tree_new(eTreeExprLoad, node, expr->type);
    self->load = expr;
    return self;
}

tree_t *tree_expr_unary(const node_t *node, unary_t unary, tree_t *expr)
{
    CTASSERT(expr != NULL);

    tree_t *self = tree_new(eTreeExprUnary, node, expr->type);
    self->unary = unary;
    self->operand = expr;
    return self;
}

tree_t *tree_expr_binary(const node_t *node, const tree_t *type, binary_t binary, tree_t *lhs, tree_t *rhs)
{
    CTASSERT(lhs != NULL);
    CTASSERT(rhs != NULL);

    tree_t *self = tree_new(eTreeExprBinary, node, type);
    self->binary = binary;
    self->lhs = lhs;
    self->rhs = rhs;
    return self;
}

tree_t *tree_expr_compare(const node_t *node, const tree_t *type, compare_t compare, tree_t *lhs, tree_t *rhs)
{
    CTASSERT(lhs != NULL);
    CTASSERT(rhs != NULL);

    tree_t *self = tree_new(eTreeExprCompare, node, type);
    self->compare = compare;
    self->lhs = lhs;
    self->rhs = rhs;
    return self;
}

tree_t *tree_expr_call(const node_t *node, const tree_t *callee, vector_t *args)
{
    CTASSERT(callee != NULL);
    CTASSERT(args != NULL);

    tree_t *self = tree_new(eTreeExprCall, node, callee->result);
    self->callee = callee;
    self->args = args;
    return self;
}

///
/// statements
///

tree_t *tree_stmt_block(const node_t *node, vector_t *stmts)
{
    CTASSERT(stmts != NULL);

    tree_t *self = tree_new(eTreeStmtBlock, node, NULL);
    self->stmts = stmts;
    return self;
}

tree_t *tree_stmt_return(const node_t *node, const tree_t *value)
{
    tree_t *self = tree_new(eTreeStmtReturn, node, value->type);
    self->value = value;
    return self;
}

tree_t *tree_stmt_assign(const node_t *node, tree_t *dst, tree_t *src)
{
    CTASSERT(dst != NULL);
    CTASSERT(src != NULL);

    tree_t *self = tree_new(eTreeStmtAssign, node, dst->type);
    self->dst = dst;
    self->src = src;
    return self;
}

tree_t *tree_stmt_loop(const node_t *node, tree_t *cond, tree_t *body, tree_t *other)
{
    CTASSERT(cond != NULL);
    CTASSERT(body != NULL);

    tree_t *self = tree_new(eTreeStmtLoop, node, NULL);
    self->cond = cond;
    self->then = body;
    self->other = other;
    return self;
}

tree_t *tree_stmt_branch(const node_t *node, tree_t *cond, tree_t *then, tree_t *other)
{
    CTASSERT(cond != NULL);
    CTASSERT(then != NULL);

    tree_t *self = tree_new(eTreeStmtBranch, node, NULL);
    self->cond = cond;
    self->then = then;
    self->other = other;
    return self;
}
