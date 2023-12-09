#include "common.h"

#include "cthulhu/tree/query.h"

#include "report/report.h"

#include "std/vector.h"
#include "std/str.h"

#include "memory/memory.h"

#include "base/panic.h"

static const tree_attribs_t kDefaultAttrib = {
    .link = eLinkModule,
    .visibility = eVisiblePrivate
};

tree_t *tree_new(tree_kind_t kind, const node_t *node, const tree_t *type)
{
    tree_t *self = MEM_ALLOC(sizeof(tree_t), tree_kind_to_string(kind), NULL);

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

void tree_report(reports_t *reports, const tree_t *error)
{
    report(reports, eFatal, tree_get_node(error), "%s", error->message);
}

static tree_t *error_vformat(const node_t *node, const char *message, va_list args)
{
    tree_t *self = tree_new(eTreeError, node, NULL);
    self->type = self;
    self->message = vformat(message, args);
    return self;
}

tree_t *tree_error(const node_t *node, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    tree_t *self = error_vformat(node, message, args);
    va_end(args);
    return self;
}

tree_t *tree_raise(const node_t *node, reports_t *reports, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    tree_t *self = error_vformat(node, message, args);
    va_end(args);

    tree_report(reports, self);

    return self;
}

///
/// types
///

static bool is_type(tree_kind_t kind)
{
    switch (kind)
    {
    case eTreeType:
    case eTreeTypeEmpty:
    case eTreeTypeUnit:
    case eTreeTypeBool:
    case eTreeTypeDigit:
    case eTreeTypeClosure:
    case eTreeTypePointer:
    case eTreeTypeOpaque:
    case eTreeTypeArray:
    case eTreeError:
    case eTreeTypeStruct:
    case eTreeTypeUnion:
    case eTreeTypeEnum:
        return true;

    default:
        return false;
    }
}

static bool is_load_type(tree_kind_t kind)
{
    switch (kind)
    {
    case eTreeError:
    case eTreeTypePointer:
    case eTreeTypeArray:
    case eTreeTypeReference:
        return true;

    default:
        return false;
    }
}

#define EXPECT_TYPE(TYPE) CTASSERTF(is_type(tree_get_kind(TYPE)), "expected type, found %s", tree_to_string(TYPE))

#define EXPECT_LOAD_TYPE(TYPE) CTASSERTF(is_load_type(tree_get_kind(TYPE)), "expected load type, found %s", tree_to_string(TYPE))

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

tree_t *tree_type_opaque(const node_t *node, const char *name)
{
    return tree_decl(eTreeTypeOpaque, node, NULL, name, eQualUnknown);
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
    EXPECT_TYPE(result);
    CTASSERT(params != NULL);

    size_t len = vector_len(params);
    for (size_t i = 0; i < len; i++)
    {
        const tree_t *param = vector_get(params, i);
        TREE_EXPECT(param, eTreeDeclParam);
    }

    tree_t *self = tree_decl(eTreeTypeClosure, node, NULL, name, eQualUnknown);
    self->result = result;
    self->params = params;
    self->arity = arity;
    return self;
}

tree_t *tree_type_pointer(const node_t *node, const char *name, const tree_t *pointer, size_t length)
{
    EXPECT_TYPE(pointer);

    tree_t *self = tree_decl(eTreeTypePointer, node, NULL, name, eQualUnknown);
    self->ptr = pointer;
    self->length = length;
    return self;
}

tree_t *tree_type_array(const node_t *node, const char *name, const tree_t *array, size_t length)
{
    EXPECT_TYPE(array);

    tree_t *self = tree_decl(eTreeTypeArray, node, NULL, name, eQualUnknown);
    self->ptr = array;
    self->length = length;
    return self;
}

tree_t *tree_type_reference(const node_t *node, const char *name, const tree_t *reference)
{
    EXPECT_TYPE(reference);

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
#define TREE_EXPECT_ADDRESS(TYPE) CTASSERTF(tree_is(TYPE, eTreeTypePointer) || tree_is(TYPE, eTreeTypeReference) || tree_is(TYPE, eTreeTypeOpaque) || tree_is(TYPE, eTreeError), "expected reference or pointer, found %s", tree_to_string(TYPE))

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
    EXPECT_LOAD_TYPE(type);

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

static const tree_t *get_ref_inner(const tree_t *ty)
{
    if (tree_is(ty, eTreeTypeReference) || tree_is(ty, eTreeTypePointer)) { return ty->ptr; }
    return ty;
}

tree_t *tree_expr_address(const node_t *node, tree_t *expr)
{
    const tree_t *type = tree_get_type(expr);
    tree_t *inner = tree_type_pointer(node, tree_get_name(type), get_ref_inner(type), 1);
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

tree_t *tree_expr_field(const node_t *node, const tree_t *type, tree_t *object, tree_t *field)
{
    const tree_t *outer = tree_get_type(object);
    const tree_t *ty = get_ref_inner(outer);

    CTASSERTF(tree_ty_is_address(outer), "object must be an address, found %s", tree_to_string(outer));
    CTASSERTF(tree_is(ty, eTreeTypeStruct), "object must be an aggregate, found %s", tree_to_string(ty));

    tree_t *self = tree_new(eTreeExprField, node, type);
    self->object = object;
    self->field = field;
    return self;
}

tree_t *tree_expr_offset(const node_t *node, const tree_t *type, tree_t *object, tree_t *offset)
{
    CTASSERT(object != NULL);
    CTASSERT(offset != NULL);

    tree_t *self = tree_new(eTreeExprOffset, node, type);
    self->object = object;
    self->offset = offset;
    return self;
}

tree_t *tree_expr_call(const node_t *node, const tree_t *callee, vector_t *args)
{
    CTASSERT(callee != NULL);
    CTASSERT(args != NULL);

    tree_t *self = tree_new(eTreeExprCall, node, tree_fn_get_return(tree_get_type(callee)));
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

tree_t *tree_stmt_jump(const node_t *node, tree_t *label, tree_jump_t jump)
{
    CTASSERT(label != NULL);
    CTASSERTF(tree_is(label, eTreeStmtLoop), "label must be a loop, found %s", tree_to_string(label));

    tree_t *self = tree_new(eTreeStmtJump, node, NULL);
    self->label = label;
    self->jump = jump;
    return self;
}
