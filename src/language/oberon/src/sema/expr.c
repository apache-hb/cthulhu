// SPDX-License-Identifier: GPL-3.0-only

#include "oberon/sema/expr.h"
#include "oberon/sema/sema.h"
#include "oberon/sema/type.h"

#include "cthulhu/util/util.h"
#include "cthulhu/util/types.h"

#include "cthulhu/events/events.h"

#include "memory/memory.h"

#include "std/vector.h"
#include "std/str.h"

#include "base/panic.h"

#include "core/macros.h"

///
/// rvalues
///

static tree_t *sema_digit(tree_t *sema, obr_t *expr, const tree_t *implicit_type)
{
    CT_UNUSED(sema);

    // TODO: get correct digit size
    const tree_t *type = implicit_type == NULL
        ? obr_get_integer_type()
        : implicit_type;

    return tree_expr_digit(expr->node, type, expr->digit);
}

static tree_t *sema_string(tree_t *sema, obr_t *expr)
{
    CT_UNUSED(sema);

    tree_t *str = obr_get_string_type(expr->node, expr->length);
    return tree_expr_string(expr->node, str, expr->text, expr->length);
}

static tree_t *sema_unary(tree_t *sema, obr_t *expr, const tree_t *implicit_type)
{
    tree_t *operand = obr_sema_rvalue(sema, expr->expr, implicit_type);
    return tree_expr_unary(expr->node, expr->unary, operand);
}

static tree_t *sema_binary(tree_t *sema, obr_t *expr, const tree_t *implicit_type)
{
    // TODO: get common type
    const tree_t *type = implicit_type == NULL
        ? obr_get_integer_type()
        : implicit_type;

    tree_t *lhs = obr_sema_rvalue(sema, expr->lhs, implicit_type);
    tree_t *rhs = obr_sema_rvalue(sema, expr->rhs, implicit_type);

    return tree_expr_binary(expr->node, type, expr->binary, lhs, rhs);
}

static tree_t *sema_compare(tree_t *sema, obr_t *expr)
{
    tree_t *lhs = obr_sema_rvalue(sema, expr->lhs, NULL);
    tree_t *rhs = obr_sema_rvalue(sema, expr->rhs, NULL);

    if (!util_types_comparable(tree_get_cookie(sema), lhs, rhs))
    {
        return tree_raise(expr->node, sema->reports, &kEvent_InvalidBinaryOperation, "cannot compare types %s and %s", tree_to_string(lhs), tree_to_string(rhs));
    }

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

    return tree_raise(expr->node, sema->reports, &kEvent_SymbolNotFound, "unknown name `%s`", expr->object);
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

static const tree_t *get_param_checked(const vector_t *params, size_t i)
{
    if (i >= vector_len(params))
    {
        return NULL;
    }

    tree_t *param = vector_get(params, i);
    return tree_get_type(param);
}

static tree_t *sema_call(tree_t *sema, obr_t *expr)
{
    tree_t *callee = obr_sema_lvalue(sema, expr->expr);
    if (tree_is(callee, eTreeError)) { return callee; }

    const vector_t *params = tree_fn_get_params(callee);

    arena_t *arena = get_global_arena();
    size_t len = vector_len(expr->args);
    vector_t *args = vector_of(len, arena);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *it = vector_get(expr->args, i);
        const tree_t *expected = get_param_checked(params, i);
        tree_t *arg = obr_sema_rvalue(sema, it, expected); // funky
        tree_t *casted = obr_cast_type(arg, expected);
        vector_set(args, i, casted);
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
        return tree_raise(expr->node, sema->reports, &kEvent_InvalidIndirection, "cannot access field of non-struct type %s", tree_to_string(decl));
    }

    tree_t *field = tree_ty_get_field(resolved, expr->object);
    if (field == NULL)
    {
        return tree_raise(expr->node, sema->reports, &kEvent_FieldNotFound, "struct %s has no field %s", tree_to_string(decl), expr->object);
    }

    return tree_expr_field(expr->node, tree_get_type(field), decl, field);
}

tree_t *obr_sema_rvalue(tree_t *sema, obr_t *expr, const tree_t *implicit_type)
{
    const tree_t *type = implicit_type != NULL ? tree_ty_load_type(tree_resolve(tree_get_cookie(sema), implicit_type)) : NULL;

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

    default: CT_NEVER("unknown expr kind %d", expr->kind);
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
    default: CT_NEVER("unknown expr kind %d", expr->kind);
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

    default: CT_NEVER("obr-default-value unknown type kind %s", tree_to_string(type));
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

static tree_t *sema_block(tree_t *sema, obr_t *stmt)
{
    const size_t sizes[eObrTagTotal] = {
        [eObrTagValues] = 4,
        [eObrTagTypes] = 4,
        [eObrTagProcs] = 4,
        [eObrTagModules] = 4,
        [eObrTagImports] = 4,
    };

    tree_t *ctx = tree_module(sema, stmt->node, NULL, eObrTagTotal, sizes);

    return obr_sema_stmts(ctx, stmt->node, stmt->stmts);
}

static tree_t *sema_stmt(tree_t *sema, obr_t *stmt);

static tree_t *sema_branch(tree_t *sema, obr_t *stmt)
{
    tree_t *cond = obr_sema_rvalue(sema, stmt->branch, obr_get_bool_type());
    tree_t *then = obr_sema_stmts(sema, stmt->node, stmt->branch_body);
    tree_t *els = stmt->branch_else == NULL ? NULL : sema_stmt(sema, stmt->branch_else);

    return tree_stmt_branch(stmt->node, cond, then, els);
}

static tree_t *sema_loop(tree_t *sema, obr_t *stmt)
{
    tree_t *body = obr_sema_stmts(sema, stmt->node, stmt->loop);

    tree_t *truthy = tree_expr_bool(stmt->node, obr_get_bool_type(), true);

    return tree_stmt_loop(stmt->node, truthy, body, NULL);
}

static tree_t *sema_while(tree_t *sema, obr_t *stmt)
{
    tree_t *cond = obr_sema_rvalue(sema, stmt->cond, obr_get_bool_type());
    tree_t *body = obr_sema_stmts(sema, stmt->node, stmt->then);

    return tree_stmt_loop(stmt->node, cond, body, NULL);
}

static tree_t *sema_repeat(tree_t *sema, obr_t *stmt)
{
    tree_t *cond = obr_sema_rvalue(sema, stmt->until, obr_get_bool_type());
    tree_t *body = obr_sema_stmts(sema, stmt->node, stmt->repeat);

    // we lower a repeat until loop into a while loop and an initial body

    arena_t *arena = get_global_arena();
    vector_t *stmts = vector_of(2, arena);
    vector_push(&stmts, body);

    tree_t *loop = tree_stmt_loop(stmt->node, cond, body, NULL);
    vector_push(&stmts, loop);

    return tree_stmt_block(stmt->node, stmts);
}

static tree_t *sema_stmt(tree_t *sema, obr_t *stmt)
{
    switch (stmt->kind)
    {
    case eObrStmtAssign: return sema_assign(sema, stmt);
    case eObrStmtReturn: return sema_return(sema, stmt);
    case eObrStmtLoop: return sema_loop(sema, stmt);
    case eObrStmtWhile: return sema_while(sema, stmt);
    case eObrStmtRepeat: return sema_repeat(sema, stmt);
    case eObrExprCall: return sema_call(sema, stmt);
    case eObrStmtBlock: return sema_block(sema, stmt);
    case eObrStmtBranch: return sema_branch(sema, stmt);
    default: CT_NEVER("unknown stmt kind %d", stmt->kind);
    }
}

tree_t *obr_sema_stmts(tree_t *sema, const node_t *node, vector_t *stmts)
{
    arena_t *arena = get_global_arena();
    size_t len = vector_len(stmts);
    vector_t *result = vector_of(len, arena);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *stmt = vector_get(stmts, i);
        tree_t *tree = sema_stmt(sema, stmt);
        vector_set(result, i, tree);
    }

    return tree_stmt_block(node, result);
}
