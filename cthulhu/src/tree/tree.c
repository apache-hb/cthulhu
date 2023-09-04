#include "common.h"

#include "cthulhu/tree/query.h"

#include "report/report.h"

#include "std/str.h"

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

tree_t *tree_decl(tree_kind_t kind, const node_t *node, const tree_t *type, const char *name, quals_t quals)
{
    CTASSERT(name != NULL);

    tree_t *self = tree_new(kind, node, type);

    self->name = name;
    self->attrib = &kDefaultAttrib;
    self->resolve = NULL;
    self->quals = quals;

    return self;
}

static tree_t *error_format(const node_t *node, const char *message, va_list args)
{
    tree_t *self = tree_new(eTreeError, node, NULL);
    self->type = self;
    self->message = formatv(message, args);
    return self;
}

tree_t *tree_error(const node_t *node, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    tree_t *self = error_format(node, message, args);
    va_end(args);
    return self;
}

tree_t *tree_raise(const node_t *node, reports_t *reports, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    tree_t *self = error_format(node, message, args);
    va_end(args);

    report(reports, eFatal, node, "%s", self->message);

    return self;
}

///
/// types
///

tree_t *tree_type_empty(const node_t *node, const char *name)
{
    return tree_decl(eTreeTypeUnit, node, NULL, name, eQualUnknown);
}

tree_t *tree_type_unit(const node_t *node, const char *name)
{
    return tree_decl(eTreeTypeUnit, node, NULL, name, eQualUnknown);
}

tree_t *tree_type_bool(const node_t *node, const char *name, quals_t quals)
{
    return tree_decl(eTreeTypeBool, node, NULL, name, quals);
}

tree_t *tree_type_digit(const node_t *node, const char *name, digit_t digit, sign_t sign, quals_t quals)
{
    tree_t *self = tree_decl(eTreeTypeDigit, node, NULL, name, quals);
    self->digit = digit;
    self->sign = sign;
    return self;
}

tree_t *tree_type_closure(const node_t *node, const char *name, const tree_t *result, vector_t *params, arity_t arity)
{
    CTASSERT(result != NULL);
    CTASSERT(params != NULL);

    tree_t *self = tree_decl(eTreeTypeClosure, node, NULL, name, eQualUnknown);
    self->result = result;
    self->params = params;
    self->arity = arity;
    return self;
}

tree_t *tree_type_pointer(const node_t *node, const char *name, const tree_t *pointer, size_t length)
{
    CTASSERT(pointer != NULL);

    tree_t *self = tree_decl(eTreeTypePointer, node, NULL, name, eQualUnknown);
    self->ptr = pointer;
    self->length = length;
    return self;
}

tree_t *tree_type_reference(const node_t *node, const char *name, const tree_t *reference)
{
    CTASSERT(reference != NULL);

    tree_t *self = tree_decl(eTreeTypeReference, node, NULL, name, eQualUnknown);
    self->ptr = reference;
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
    TREE_EXPECT(type, eTreeTypeBool);

    tree_t *self = tree_new(eTreeExprBool, node, type);
    self->boolValue = value;
    return self;
}

tree_t *tree_expr_digit(const node_t *node, const tree_t *type, const mpz_t value)
{
    TREE_EXPECT(type, eTreeTypeDigit);

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

/// must be either storage or a pointer
#define TREE_EXPECT_ADDRESS(TYPE) CTASSERTF(tree_is(TYPE, eTreeTypePointer) || tree_is(TYPE, eTreeTypeReference) || tree_is(TYPE, eTreeError), "expected reference or pointer, found %s", tree_to_string(TYPE))

tree_t *tree_expr_cast(const node_t *node, const tree_t *type, tree_t *expr)
{
    CTASSERT(expr != NULL);

    tree_t *self = tree_new(eTreeExprCast, node, type);
    self->cast = expr;
    return self;
}

tree_t *tree_expr_load(const node_t *node, tree_t *expr)
{
    const tree_t *type = tree_get_type(expr);
    TREE_EXPECT_ADDRESS(type);

    tree_t *self = tree_new(eTreeExprLoad, node, tree_ty_load_type(type));
    self->load = expr;
    return self;
}

tree_t *tree_expr_ref(const node_t *node, tree_t *expr)
{
    const tree_t *type = tree_get_type(expr);
    tree_t *inner = tree_type_reference(node, tree_get_name(type), tree_ty_load_type(type));
    tree_t *self = tree_new(eTreeExprReference, node, inner);
    self->expr = expr;
    return self;
}

tree_t *tree_expr_address(const node_t *node, tree_t *expr)
{
    const tree_t *type = tree_get_type(expr);
    tree_t *inner = tree_type_pointer(node, tree_get_name(type), type, 1);
    tree_t *self = tree_new(eTreeExprAddress, node, inner);
    self->expr = expr;
    return self;
}

tree_t *tree_expr_unary(const node_t *node, unary_t unary, tree_t *expr)
{
    tree_t *self = tree_new(eTreeExprUnary, node, tree_get_type(expr));
    self->unary = unary;
    self->operand = expr;
    return self;
}

tree_t *tree_expr_binary(const node_t *node, const tree_t *type, binary_t binary, tree_t *lhs, tree_t *rhs)
{
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

tree_t *tree_expr_field(const node_t *node, tree_t *object, tree_t *field)
{
    CTASSERTF(tree_is(object, eTreeTypeStruct), "object must be an aggregate, found %s", tree_to_string(object));

    tree_t *self = tree_new(eTreeExprField, node, tree_get_type(field));
    self->object = object;
    self->field = field;
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
    CTASSERT(value != NULL);

    tree_t *self = tree_new(eTreeStmtReturn, node, NULL);
    self->value = value;
    return self;
}

tree_t *tree_stmt_assign(const node_t *node, tree_t *dst, tree_t *src)
{
    const tree_t *dstType = tree_get_type(dst);
    TREE_EXPECT_ADDRESS(dstType);

    CTASSERT(src != NULL);

    tree_t *self = tree_new(eTreeStmtAssign, node, NULL);
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

tree_t *tree_decl_storage(
    const node_t *node, const char *name,
    const tree_t *type, size_t length, quals_t quals)
{
    CTASSERT(type != NULL);

    tree_t *self = tree_decl(eTreeDeclStorage, node, type, name, eQualUnknown);
    self->length = length;
    self->quals = quals;
    return self;
}
