#include "oberon/sema/expr.h"
#include "oberon/sema/type.h"

#include "cthulhu/util/util.h"

#include "std/str.h"

#include "base/panic.h"

///
/// rvalues
///

static tree_t *sema_digit(tree_t *sema, obr_t *expr, const tree_t *implicitType)
{
    // TODO: get correct digit size
    const tree_t *type = implicitType != NULL ? implicitType : obr_get_digit_type(eDigitInt, eSignSigned);
    return tree_expr_digit(expr->node, type, expr->digit);
}

static tree_t *sema_string(tree_t *sema, obr_t *expr)
{
    const node_t *node = tree_get_node(sema);
    const tree_t *type = obr_get_string_type(expr->length + 1);
    return tree_expr_string(node, type, expr->text, expr->length + 1);;
}

static tree_t *sema_unary(tree_t *sema, obr_t *expr, const tree_t *implicitType)
{
    tree_t *operand = obr_sema_rvalue(sema, expr->expr, implicitType);
    return tree_expr_unary(expr->node, expr->unary, operand);
}

static tree_t *sema_binary(tree_t *sema, obr_t *expr, const tree_t *implicitType)
{
    // TODO: get common type
    const tree_t *type = implicitType == NULL
        ? obr_get_digit_type(eDigitInt, eSignSigned)
        : implicitType;

    tree_t *lhs = obr_sema_rvalue(sema, expr->lhs, implicitType);
    tree_t *rhs = obr_sema_rvalue(sema, expr->rhs, implicitType);

    return tree_expr_binary(expr->node, type, expr->binary, lhs, rhs);
}

static tree_t *sema_compare(tree_t *sema, obr_t *expr)
{
    // TODO: check types are comparable

    tree_t *lhs = obr_sema_rvalue(sema, expr->lhs, NULL);
    tree_t *rhs = obr_sema_rvalue(sema, expr->rhs, NULL);

    return tree_expr_compare(expr->node, obr_get_bool_type(), expr->compare, lhs, rhs);
}

static tree_t *sema_name(tree_t *sema, obr_t *expr)
{
    tree_t *var = obr_get_symbol(sema, eObrTagValues, expr->object);
    if (var != NULL) { return var; }

    tree_t *fn = obr_get_symbol(sema, eObrTagProcs, expr->object);
    if (fn != NULL) { return fn; }

    tree_t *mod = obr_get_namespace(sema, expr->object);
    if (mod != NULL) { return mod; }

    return tree_raise(expr->node, sema->reports, "unknown name `%s`", expr->object);
}

static tree_t *sema_name_rvalue(tree_t *sema, obr_t *expr)
{
    tree_t *name = sema_name(sema, expr);
    if (!tree_is(name, eTreeDeclParam))
    {
        return tree_expr_load(expr->node, name);
    }

    return name;
}

static tree_t *sema_call(tree_t *sema, obr_t *expr)
{
    tree_t *callee = obr_sema_lvalue(sema, expr->expr);
    if (tree_is(callee, eTreeError)) { return callee; }

    vector_t *params = tree_fn_get_params(callee);

    size_t len = vector_len(expr->args);
    vector_t *args = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *it = vector_get(expr->args, i);
        tree_t *expected = vector_get(params, i);
        tree_t *arg = obr_sema_rvalue(sema, it, (tree_t*)tree_get_type(expected)); // funky
        vector_set(args, i, arg);
    }

    return tree_expr_call(expr->node, callee, args);
}

static tree_t *sema_field(tree_t *sema, obr_t *expr)
{
    tree_t *decl = obr_sema_lvalue(sema, expr->expr);
    switch(tree_get_kind(decl))
    {
    case eTreeDeclModule: return sema_name(decl, expr);
    case eTreeError: return decl;
    default: break;
    }

    const tree_t *resolved = tree_resolve(tree_get_cookie(sema), tree_get_type(decl));
    if (!tree_is(resolved, eTreeTypeStruct))
    {
        return tree_raise(expr->node, sema->reports, "cannot access field of non-struct type %s", tree_to_string(decl));
    }

    tree_t *field = tree_ty_get_field(resolved, expr->object);
    if (field == NULL)
    {
        return tree_raise(expr->node, sema->reports, "struct %s has no field %s", tree_to_string(decl), expr->object);
    }

    return tree_expr_field(expr->node, decl, field);
}

tree_t *obr_sema_rvalue(tree_t *sema, obr_t *expr, const tree_t *implicitType)
{
    const tree_t *type = implicitType != NULL ? tree_ty_load_type(tree_resolve(tree_get_cookie(sema), implicitType)) : NULL;

    switch (expr->kind)
    {
    case eObrExprDigit: return sema_digit(sema, expr, type);
    case eObrExprString: return sema_string(sema, expr);
    case eObrExprUnary: return sema_unary(sema, expr, type);
    case eObrExprBinary: return sema_binary(sema, expr, type);
    case eObrExprCompare: return sema_compare(sema, expr);
    case eObrExprName: return sema_name_rvalue(sema, expr); // TODO: this feels wrong for functions
    case eObrExprField: return tree_expr_load(expr->node, sema_field(sema, expr)); // TODO: may also be wrong for functions
    case eObrExprCall: return sema_call(sema, expr);

    default: NEVER("unknown expr kind %d", expr->kind);
    }
}

///
/// lvalues
///

tree_t *obr_sema_lvalue(tree_t *sema, obr_t *expr)
{
    switch (expr->kind)
    {
    case eObrExprName: return sema_name(sema, expr);
    case eObrExprField: return sema_field(sema, expr);
    default: NEVER("unknown expr kind %d", expr->kind);
    }
}

///
/// default values
///

tree_t *obr_default_value(const node_t *node, const tree_t *type)
{
    switch (tree_get_kind(type))
    {
    case eTreeTypeBool: return tree_expr_bool(node, type, false);
    case eTreeTypeUnit: return tree_expr_unit(node, type);

    case eTreeTypeDigit: {
        mpz_t zero;
        mpz_init(zero);
        return tree_expr_digit(node, type, zero);
    }

    case eTreeTypeReference: return obr_default_value(node, tree_ty_load_type(type));

    default: NEVER("obr-default-value unknown type kind %s", tree_to_string(type));
    }
}

///
/// statements
///

static tree_t *sema_assign(tree_t *sema, obr_t *stmt)
{
    tree_t *dst = obr_sema_lvalue(sema, stmt->dst);
    tree_t *src = obr_sema_rvalue(sema, stmt->src, (tree_t*)tree_get_type(dst)); // TODO: a little evil cast

    return tree_stmt_assign(stmt->node, dst, src);
}

static tree_t *sema_return(tree_t *sema, obr_t *stmt)
{
    // TODO: get implicit return type
    tree_t *value = stmt->expr == NULL
        ? tree_expr_unit(stmt->node, obr_get_void_type())
        : obr_sema_rvalue(sema, stmt->expr, NULL);
    return tree_stmt_return(stmt->node, value);
}

static tree_t *sema_stmt(tree_t *sema, obr_t *stmt)
{
    switch (stmt->kind)
    {
    case eObrStmtAssign: return sema_assign(sema, stmt);
    case eObrStmtReturn: return sema_return(sema, stmt);
    case eObrExprCall: return sema_call(sema, stmt);
    default: NEVER("unknown stmt kind %d", stmt->kind);
    }
}

tree_t *obr_sema_stmts(tree_t *sema, const node_t *node, const char *name, vector_t *stmts)
{
    size_t len = vector_len(stmts);
    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *stmt = vector_get(stmts, i);
        tree_t *tree = sema_stmt(sema, stmt);
        vector_set(result, i, tree);
    }

    return tree_stmt_block(node, result);
}
