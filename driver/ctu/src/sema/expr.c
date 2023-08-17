#include "ctu/sema/expr.h"
#include "ctu/sema/type.h"

#include "cthulhu/hlir/query.h"

#include "ctu/ast.h"

#include "report/report.h"

#include "std/vector.h"

#include "base/panic.h"

///
/// get decls
///

static h2_t *sema_decl_name(h2_t *sema, const node_t *node, vector_t *path)
{
    size_t len = vector_len(path);
    h2_t *ns = sema;
    for (size_t i = 0; i < len - 1; i++)
    {
        const char *segment = vector_get(path, i);
        ns = ctu_get_namespace(ns, segment);
        if (ns == NULL)
        {
            report(sema->reports, eFatal, NULL, "namespace `%s` not found", segment);
            return h2_error(node, "namespace not found");
        }
    }

    const char *name = vector_tail(path);
    h2_t *decl = ctu_get_decl(ns, name);
    if (decl == NULL)
    {
        report(sema->reports, eFatal, NULL, "decl `%s` not found", name);
        return h2_error(node, "decl not found");
    }

    return decl;
}

///
/// inner logic
///

static h2_t *verify_expr_type(h2_t *sema, h2_kind_t kind, const h2_t *type, const char *exprKind, const node_t *node)
{
    if (type == NULL) { return NULL; }
    if (ctu_type_is(type, kind)) { return NULL; }

    report(sema->reports, eFatal, node, "%ss are not implicitly convertable to `%s`", exprKind, ctu_type_string(type));
    return h2_error(node, "invalid implicit type conversion");
}

static h2_t *ctu_sema_bool(h2_t *sema, const ctu_t *expr, const h2_t *implicitType)
{
    const h2_t *type = implicitType ? implicitType : ctu_get_bool_type();

    h2_t *verify = verify_expr_type(sema, eHlir2TypeBool, type, "boolean literal", expr->node);
    if (verify != NULL) { return verify; }

    return h2_expr_bool(expr->node, type, expr->boolValue);
}

static h2_t *ctu_sema_int(h2_t *sema, const ctu_t *expr, const h2_t *implicitType)
{
    const h2_t *type = implicitType ? implicitType : ctu_get_int_type(eDigitInt, eSignSigned); // TODO: calculate proper type to use

    h2_t *verify = verify_expr_type(sema, eHlir2TypeDigit, type, "integer literal", expr->node);
    if (verify != NULL) { return verify; }

    return h2_expr_digit(expr->node, type, expr->intValue);
}

static h2_t *ctu_sema_name(h2_t *sema, const ctu_t *expr, const h2_t *implicitType)
{
    h2_t *decl = sema_decl_name(sema, expr->node, expr->path);
    if (h2_is(decl, eHlir2Error))
    {
        report(sema->reports, eFatal, expr->node, "name `%s` not found", expr->name);
        return h2_error(expr->node, "name not found");
    }

    return h2_resolve(h2_get_cookie(sema), decl);
}

h2_t *ctu_sema_lvalue(h2_t *sema, const ctu_t *expr, h2_t *implicitType)
{
    CTASSERT(expr != NULL);

    switch (expr->kind)
    {
    case eCtuExprName: return ctu_sema_name(sema, expr, implicitType);

    default: NEVER("invalid lvalue-expr kind %d", expr->kind);
    }
}

h2_t *ctu_sema_rvalue(h2_t *sema, const ctu_t *expr, h2_t *implicitType)
{
    CTASSERT(expr != NULL);

    h2_t *inner = implicitType == NULL ? NULL : h2_resolve(h2_get_cookie(sema), implicitType);

    switch (expr->kind)
    {
    case eCtuExprBool: return ctu_sema_bool(sema, expr, inner);
    case eCtuExprInt: return ctu_sema_int(sema, expr, inner);
    case eCtuExprName: return h2_expr_load(expr->node, ctu_sema_name(sema, expr, implicitType));

    default: NEVER("invalid rvalue-expr kind %d", expr->kind);
    }
}

static h2_t *ctu_sema_local(h2_t *sema, h2_t *decl, const ctu_t *stmt)
{
    h2_t *type = stmt->type == NULL ? NULL : ctu_sema_type(sema, stmt->type);
    h2_t *value = stmt->value == NULL ? NULL : ctu_sema_rvalue(sema, stmt->value, type);

    CTASSERT(value != NULL || type != NULL);

    h2_t *self = h2_decl_local(decl->node, stmt->name, type);

    if (value != NULL)
    {
        return h2_stmt_assign(stmt->node, self, value);
    }

    return h2_stmt_block(stmt->node, vector_of(0)); // TODO: good enough
}

static h2_t *ctu_sema_stmts(h2_t *sema, h2_t *decl, const ctu_t *stmt)
{
    size_t len = vector_len(stmt->stmts);
    vector_t *stmts = vector_of(len);

    size_t sizes[eCtuTagTotal] = {
        [eCtuTagTypes] = 4,
        [eCtuTagValues] = 4,
        [eCtuTagFunctions] = 4
    };

    h2_t *ctx = h2_module(sema, stmt->node, decl->name, eCtuTagTotal, sizes);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *it = vector_get(stmt->stmts, i);
        h2_t *step = ctu_sema_stmt(ctx, decl, it);
        vector_set(stmts, i, step);
    }

    return h2_stmt_block(stmt->node, stmts);
}

h2_t *ctu_sema_stmt(h2_t *sema, h2_t *decl, const ctu_t *stmt)
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
