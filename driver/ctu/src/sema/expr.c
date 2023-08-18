#include "ctu/sema/expr.h"
#include "ctu/sema/type.h"

#include "cthulhu/tree/query.h"

#include "ctu/ast.h"

#include "report/report.h"

#include "std/vector.h"

#include "base/panic.h"

///
/// get decls
///

static tree_t *sema_decl_name(tree_t *sema, const node_t *node, vector_t *path)
{
    size_t len = vector_len(path);
    tree_t *ns = sema;
    for (size_t i = 0; i < len - 1; i++)
    {
        const char *segment = vector_get(path, i);
        ns = ctu_get_namespace(ns, segment);
        if (ns == NULL)
        {
            report(sema->reports, eFatal, NULL, "namespace `%s` not found", segment);
            return tree_error(node, "namespace not found");
        }
    }

    const char *name = vector_tail(path);
    tree_t *decl = ctu_get_decl(ns, name);
    if (decl == NULL)
    {
        report(sema->reports, eFatal, NULL, "decl `%s` not found", name);
        return tree_error(node, "decl not found");
    }

    return decl;
}

///
/// inner logic
///

static tree_t *verify_expr_type(tree_t *sema, tree_kind_t kind, const tree_t *type, const char *exprKind, const node_t *node)
{
    if (type == NULL) { return NULL; }
    if (ctu_type_is(type, kind)) { return NULL; }

    report(sema->reports, eFatal, node, "%ss are not implicitly convertable to `%s`", exprKind, ctu_type_string(type));
    return tree_error(node, "invalid implicit type conversion");
}

static tree_t *ctu_sema_bool(tree_t *sema, const ctu_t *expr, const tree_t *implicitType)
{
    const tree_t *type = implicitType ? implicitType : ctu_get_bool_type();

    tree_t *verify = verify_expr_type(sema, eTreeTypeBool, type, "boolean literal", expr->node);
    if (verify != NULL) { return verify; }

    return tree_expr_bool(expr->node, type, expr->boolValue);
}

static tree_t *ctu_sema_int(tree_t *sema, const ctu_t *expr, const tree_t *implicitType)
{
    const tree_t *type = implicitType ? implicitType : ctu_get_int_type(eDigitInt, eSignSigned); // TODO: calculate proper type to use

    tree_t *verify = verify_expr_type(sema, eTreeTypeDigit, type, "integer literal", expr->node);
    if (verify != NULL) { return verify; }

    return tree_expr_digit(expr->node, type, expr->intValue);
}

static tree_t *ctu_sema_name(tree_t *sema, const ctu_t *expr, const tree_t *implicitType)
{
    tree_t *decl = sema_decl_name(sema, expr->node, expr->path);
    if (tree_is(decl, eTreeError))
    {
        report(sema->reports, eFatal, expr->node, "name `%s` not found", expr->name);
        return tree_error(expr->node, "name not found");
    }

    return tree_resolve(tree_get_cookie(sema), decl);
}

tree_t *ctu_sema_lvalue(tree_t *sema, const ctu_t *expr, tree_t *implicitType)
{
    CTASSERT(expr != NULL);

    switch (expr->kind)
    {
    case eCtuExprName: return ctu_sema_name(sema, expr, implicitType);

    default: NEVER("invalid lvalue-expr kind %d", expr->kind);
    }
}

tree_t *ctu_sema_rvalue(tree_t *sema, const ctu_t *expr, tree_t *implicitType)
{
    CTASSERT(expr != NULL);

    tree_t *inner = implicitType == NULL ? NULL : tree_resolve(tree_get_cookie(sema), implicitType);

    switch (expr->kind)
    {
    case eCtuExprBool: return ctu_sema_bool(sema, expr, inner);
    case eCtuExprInt: return ctu_sema_int(sema, expr, inner);
    case eCtuExprName: return tree_expr_load(expr->node, ctu_sema_name(sema, expr, implicitType));

    default: NEVER("invalid rvalue-expr kind %d", expr->kind);
    }
}

static tree_t *ctu_sema_local(tree_t *sema, tree_t *decl, const ctu_t *stmt)
{
    tree_t *type = stmt->type == NULL ? NULL : ctu_sema_type(sema, stmt->type);
    tree_t *value = stmt->value == NULL ? NULL : ctu_sema_rvalue(sema, stmt->value, type);

    CTASSERT(value != NULL || type != NULL);

    tree_t *self = tree_decl_local(decl->node, stmt->name, type);

    if (value != NULL)
    {
        return tree_stmt_assign(stmt->node, self, value);
    }

    return tree_stmt_block(stmt->node, vector_of(0)); // TODO: good enough
}

static tree_t *ctu_sema_stmts(tree_t *sema, tree_t *decl, const ctu_t *stmt)
{
    size_t len = vector_len(stmt->stmts);
    vector_t *stmts = vector_of(len);

    size_t sizes[eCtuTagTotal] = {
        [eCtuTagTypes] = 4,
        [eCtuTagValues] = 4,
        [eCtuTagFunctions] = 4
    };

    tree_t *ctx = tree_module(sema, stmt->node, decl->name, eCtuTagTotal, sizes);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *it = vector_get(stmt->stmts, i);
        tree_t *step = ctu_sema_stmt(ctx, decl, it);
        vector_set(stmts, i, step);
    }

    return tree_stmt_block(stmt->node, stmts);
}

tree_t *ctu_sema_stmt(tree_t *sema, tree_t *decl, const ctu_t *stmt)
{
    CTASSERT(decl != NULL);
    CTASSERT(stmt != NULL);

    switch (stmt->kind)
    {
    case eCtuStmtLocal:
        return ctu_sema_local(sema, decl, stmt);
    case eCtuStmtList:
        return ctu_sema_stmts(sema, decl, stmt);

    default:
        NEVER("invalid stmt kind %d", stmt->kind);
    }
}
