#include "ctu/sema/expr.h"
#include "ctu/sema/type.h"

#include "cthulhu/util/util.h"

#include "cthulhu/tree/query.h"

#include "ctu/ast.h"

#include "report/report.h"

#include "std/str.h"
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
            return tree_raise(node, sema->reports, "namespace `%s` not found", segment);
        }
    }

    const char *name = vector_tail(path);
    tree_t *decl = ctu_get_decl(ns, name);
    if (decl == NULL)
    {
        return tree_raise(node, sema->reports, "decl `%s` not found", name);
    }

    return decl;
}

///
/// inner logic
///

static tree_t *verify_expr_type(tree_t *sema, tree_kind_t kind, const tree_t *type, const char *exprKind, const node_t *node)
{
    if (type == NULL) { return NULL; }
    if (tree_is(type, kind)) { return NULL; }

    return tree_raise(node, sema->reports, "%ss are not implicitly convertable to `%s`", exprKind, tree_to_string(type));
}

static tree_t *sema_bool(tree_t *sema, const ctu_t *expr, const tree_t *implicitType)
{
    const tree_t *type = implicitType ? implicitType : ctu_get_bool_type();

    tree_t *verify = verify_expr_type(sema, eTreeTypeBool, type, "boolean literal", expr->node);
    if (verify != NULL) { return verify; }

    return tree_expr_bool(expr->node, type, expr->boolValue);
}

static tree_t *sema_int(tree_t *sema, const ctu_t *expr, const tree_t *implicitType)
{
    const tree_t *type = implicitType ? implicitType : ctu_get_int_type(eDigitInt, eSignSigned); // TODO: calculate proper type to use

    tree_t *verify = verify_expr_type(sema, eTreeTypeDigit, type, "integer literal", expr->node);
    if (verify != NULL) { return verify; }

    return tree_expr_digit(expr->node, type, expr->intValue);
}

static tree_t *sema_string(tree_t *sema, const ctu_t *expr)
{
    const node_t *node = tree_get_node(sema);
    where_t where = get_node_location(node);

    // generate a unique name for the string
    // TODO: its not really unique
    const tree_t *currentSymbol = ctu_current_symbol(sema);
    char *name = format("%s$s$%llu$%llu", tree_get_name(currentSymbol), where.firstLine, where.firstColumn);

    // create the string and put it into a global value
    const tree_t *type = ctu_get_str_type(expr->length + 1);
    tree_t *storage = tree_type_storage(node, name, ctu_get_int_type(eDigitChar, eSignSigned), expr->length + 1, eQualConst);
    tree_t *init = tree_expr_string(node, type, expr->text, expr->length + 1);
    tree_t *global = tree_decl_global(node, name, storage, init);

    // add the global to the current module
    tree_t *currentModule = util_current_module(sema);
    ctu_add_decl(currentModule, eCtuTagValues, name, global);

    // job done
    return global;
}

static tree_t *sema_name(tree_t *sema, const ctu_t *expr, const tree_t *implicitType)
{
    tree_t *decl = sema_decl_name(sema, expr->node, expr->path);
    if (tree_is(decl, eTreeError))
    {
        return tree_error(expr->node, "name not found");
    }

    return tree_resolve(tree_get_cookie(sema), decl);
}

static tree_t *sema_compare(tree_t *sema, const ctu_t *expr)
{
    tree_t *left = ctu_sema_rvalue(sema, expr->lhs, NULL);
    tree_t *right = ctu_sema_rvalue(sema, expr->rhs, NULL);

    if (tree_is(left, eTreeError) || tree_is(right, eTreeError))
    {
        return tree_error(expr->node, "invalid compare");
    }

    return tree_expr_compare(expr->node, ctu_get_bool_type(), expr->compare, left, right);
}

static tree_t *sema_binary(tree_t *sema, const ctu_t *expr, const tree_t *implicitType)
{
    tree_t *left = ctu_sema_rvalue(sema, expr->lhs, implicitType);
    tree_t *right = ctu_sema_rvalue(sema, expr->rhs, implicitType);

    if (tree_is(left, eTreeError) || tree_is(right, eTreeError))
    {
        return tree_error(expr->node, "invalid binary");
    }

    // TODO: calculate proper type to use
    const tree_t *commonType = implicitType == NULL ? tree_get_type(left) : implicitType;

    return tree_expr_binary(expr->node, commonType, expr->binary, left, right);
}

static tree_t *sema_unary(tree_t *sema, const ctu_t *expr, const tree_t *implicitType)
{
    tree_t *inner = ctu_sema_rvalue(sema, expr->expr, implicitType);

    if (tree_is(inner, eTreeError))
    {
        return tree_error(expr->node, "invalid unary");
    }

    return tree_expr_unary(expr->node, expr->unary, inner);
}

tree_t *ctu_sema_lvalue(tree_t *sema, const ctu_t *expr, const tree_t *implicitType)
{
    CTASSERT(expr != NULL);

    switch (expr->kind)
    {
    case eCtuExprName: return sema_name(sema, expr, implicitType);

    default: NEVER("invalid lvalue-expr kind %d", expr->kind);
    }
}

tree_t *ctu_sema_rvalue(tree_t *sema, const ctu_t *expr, const tree_t *implicitType)
{
    CTASSERT(expr != NULL);

    const tree_t *inner = implicitType == NULL ? NULL : tree_resolve(tree_get_cookie(sema), implicitType);

    switch (expr->kind)
    {
    case eCtuExprBool: return sema_bool(sema, expr, inner);
    case eCtuExprInt: return sema_int(sema, expr, inner);
    case eCtuExprString: return sema_string(sema, expr);

    case eCtuExprName: return tree_expr_load(expr->node, sema_name(sema, expr, implicitType));

    case eCtuExprCompare: return sema_compare(sema, expr);
    case eCtuExprBinary: return sema_binary(sema, expr, inner);
    case eCtuExprUnary: return sema_unary(sema, expr, inner);

    default: NEVER("invalid rvalue-expr kind %d", expr->kind);
    }
}

static tree_t *sema_local(tree_t *sema, tree_t *decl, const ctu_t *stmt)
{
    tree_t *type = stmt->type == NULL ? NULL : ctu_sema_type(sema, stmt->type);
    tree_t *value = stmt->value == NULL ? NULL : ctu_sema_rvalue(sema, stmt->value, type);

    CTASSERT(value != NULL || type != NULL);

    const tree_t *actualType = type != NULL
        ? tree_resolve(tree_get_cookie(sema), type)
        : tree_get_type(value);

    tree_t *store = tree_type_storage(stmt->node, stmt->name, actualType, 1, stmt->mut ? eQualMutable : eQualConst);
    tree_t *self = tree_decl_local(decl->node, stmt->name, store);
    tree_add_local(decl, self);
    ctu_add_decl(sema, eCtuTagValues, stmt->name, self);

    if (value != NULL)
    {
        return tree_stmt_assign(stmt->node, self, value);
    }

    return tree_stmt_block(stmt->node, vector_of(0)); // TODO: good enough
}

static tree_t *sema_stmts(tree_t *sema, tree_t *decl, const ctu_t *stmt)
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

static tree_t *sema_return(tree_t *sema, tree_t *decl, const ctu_t *stmt)
{
    tree_t *fn = ctu_current_symbol(sema);
    const tree_t *type = tree_get_type(fn);

    tree_t *value = stmt->result == NULL ? NULL : util_type_cast(type->result, ctu_sema_rvalue(sema, stmt->result, type->result));
    return tree_stmt_return(stmt->node, value);
}

static tree_t *sema_while(tree_t *sema, tree_t *decl, const ctu_t *stmt)
{
    tree_t *cond = ctu_sema_rvalue(sema, stmt->cond, ctu_get_bool_type());
    tree_t *then = ctu_sema_stmt(sema, decl, stmt->then);

    NEVER("TODO: implement while");

    return NULL;
}

tree_t *ctu_sema_stmt(tree_t *sema, tree_t *decl, const ctu_t *stmt)
{
    CTASSERT(decl != NULL);
    CTASSERT(stmt != NULL);

    switch (stmt->kind)
    {
    case eCtuStmtLocal: return sema_local(sema, decl, stmt);
    case eCtuStmtList: return sema_stmts(sema, decl, stmt);
    case eCtuStmtReturn: return sema_return(sema, decl, stmt);
    case eCtuStmtWhile: return sema_while(sema, decl, stmt);

    default:
        NEVER("invalid stmt kind %d", stmt->kind);
    }
}
