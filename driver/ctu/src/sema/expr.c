#include "ctu/sema/expr.h"
#include "ctu/sema/type.h"

#include "cthulhu/util/util.h"
#include "cthulhu/util/type.h"

#include "cthulhu/tree/query.h"

#include "ctu/ast.h"

#include "report/report.h"

#include "std/str.h"
#include "std/vector.h"

#include "base/panic.h"

///
/// get decls
///

static const size_t kLocalModuleTags[] = { eCtuTagModules };
static const size_t kGlobalModuleTags[] = { eCtuTagImports };
static const size_t kDeclTags[] = { eCtuTagValues, eCtuTagFunctions };

static const util_search_t kSearchName = {
    .localScopeTags = kLocalModuleTags,
    .localScopeTagsLen = sizeof(kLocalModuleTags) / sizeof(size_t),

    .globalScopeTags = kGlobalModuleTags,
    .globalScopeTagsLen = sizeof(kGlobalModuleTags) / sizeof(size_t),

    .declTags = kDeclTags,
    .declTagsLen = sizeof(kDeclTags) / sizeof(size_t)
};

static tree_t *sema_decl_name(tree_t *sema, const node_t *node, vector_t *path)
{
    return util_search_path(sema, &kSearchName, node, path);
}

///
/// inner logic
///

static tree_t *verify_expr_type(tree_t *sema, tree_kind_t kind, const tree_t *type, const char *exprKind, tree_t *expr)
{
    CTU_UNUSED(sema);
    CTU_UNUSED(kind);
    CTU_UNUSED(exprKind);

    if (type == NULL) { return expr; }

    return util_type_cast(type, expr);
}

static tree_t *sema_bool(tree_t *sema, const ctu_t *expr, const tree_t *implicitType)
{
    const tree_t *type = implicitType ? implicitType : ctu_get_bool_type();
    if (!tree_is(type, eTreeTypeBool)) { return tree_raise(expr->node, sema->reports, "invalid type `%s` for boolean literal", tree_to_string(type)); }

    tree_t *it = tree_expr_bool(expr->node, type, expr->boolValue);

    return verify_expr_type(sema, eTreeTypeBool, type, "boolean literal", it);
}

static tree_t *sema_int(tree_t *sema, const ctu_t *expr, const tree_t *implicitType)
{
    const tree_t *type = implicitType ? implicitType : ctu_get_int_type(eDigitInt, eSignSigned); // TODO: calculate proper type to use
    if (!tree_is(type, eTreeTypeDigit)) { return tree_raise(expr->node, sema->reports, "invalid type `%s` for integer literal", tree_to_string(type)); }

    tree_t *it = tree_expr_digit(expr->node, type, expr->intValue);

    return verify_expr_type(sema, eTreeTypeDigit, type, "integer literal", it);
}

static tree_t *sema_string(tree_t *sema, const ctu_t *expr)
{
    return util_create_string(sema, ctu_get_char_type(), expr->text, expr->length);
}

static tree_t *sema_name(tree_t *sema, const ctu_t *expr)
{
    tree_t *decl = sema_decl_name(sema, expr->node, expr->path);
    if (tree_is(decl, eTreeError))
    {
        return tree_error(expr->node, "name not found");
    }

    if (tree_is(decl, eTreeDeclFunction)) { return decl; }

    return tree_resolve(tree_get_cookie(sema), decl);
}

static tree_t *sema_load(tree_t *sema, const ctu_t *expr)
{
    tree_t *name = sema_name(sema, expr);
    if (tree_is(name, eTreeDeclParam) || tree_is(name, eTreeDeclFunction)) { return name; } // TODO: feels bad

    return tree_expr_load(expr->node, name);
}

static tree_t *sema_compare(tree_t *sema, const ctu_t *expr)
{
    tree_t *left = ctu_sema_rvalue(sema, expr->lhs, NULL);
    tree_t *right = ctu_sema_rvalue(sema, expr->rhs, NULL);

    if (!util_types_equal(tree_get_type(left), tree_get_type(right)))
    {
        return tree_raise(expr->node, sema->reports, "cannot compare `%s` to `%s`", tree_to_string(tree_get_type(left)), tree_to_string(tree_get_type(right)));
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

static tree_t *sema_call(tree_t *sema, const ctu_t *expr)
{
    tree_t *callee = ctu_sema_lvalue(sema, expr->callee);
    if (tree_is(callee, eTreeError)) { return callee; }

    size_t len = vector_len(expr->args);
    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *it = vector_get(expr->args, i);
        tree_t *arg = ctu_sema_rvalue(sema, it, NULL);
        vector_set(result, i, arg);
    }

    return tree_expr_call(expr->node, callee, result);
}

static tree_t *sema_deref_lvalue(tree_t *sema, const ctu_t *expr)
{
    tree_t *inner = ctu_sema_rvalue(sema, expr->expr, NULL);
    if (tree_is(inner, eTreeError)) { return inner; }

    return tree_expr_ref(expr->node, inner);
}

static tree_t *sema_deref_rvalue(tree_t *sema, const ctu_t *expr)
{
    tree_t *inner = ctu_sema_rvalue(sema, expr->expr, NULL);
    if (tree_is(inner, eTreeError)) { return inner; }

    return tree_expr_load(expr->node, inner);
}

static tree_t *sema_ref(tree_t *sema, const ctu_t *expr)
{
    tree_t *inner = ctu_sema_lvalue(sema, expr->expr);
    if (tree_is(inner, eTreeError)) { return inner; }

    return tree_expr_address(expr->node, inner);
}

static const tree_t *get_ptr_type(const tree_t *ty)
{
    if (tree_is(ty, eTreeTypeReference))
    {
        return ty->ptr;
    }

    return ty;
}

static bool can_index_type(const tree_t *ty)
{
    switch (tree_get_kind(ty))
    {
    case eTreeTypePointer:
    case eTreeTypeArray:
        return true;

    default:
        return false;
    }
}

static tree_t *sema_index_rvalue(tree_t *sema, const ctu_t *expr)
{
    tree_t *index = ctu_sema_rvalue(sema, expr->index, ctu_get_int_type(eDigitSize, eSignUnsigned));
    tree_t *object = ctu_sema_lvalue(sema, expr->expr);

    const tree_t *ty = get_ptr_type(tree_get_type(object));
    if (!can_index_type(ty))
    {
        return tree_raise(expr->node, sema->reports, "cannot index non-pointer type `%s` inside rvalue", tree_to_string(ty));
    }

    tree_t *offset = tree_expr_offset(expr->node, ty, object, index);
    return tree_expr_load(expr->node, offset);
}

static tree_t *sema_index_lvalue(tree_t *sema, const ctu_t *expr)
{
    tree_t *index = ctu_sema_rvalue(sema, expr->index, ctu_get_int_type(eDigitSize, eSignUnsigned));
    tree_t *object = ctu_sema_lvalue(sema, expr->expr);

    const tree_t *ty = get_ptr_type(tree_get_type(object));
    if (!can_index_type(ty))
    {
        return tree_raise(expr->node, sema->reports, "cannot index non-pointer type `%s` inside lvalue", tree_to_string(ty));
    }

    tree_t *ref = tree_type_reference(expr->node, "", ty->ptr);
    return tree_expr_offset(expr->node, ref, object, index);
}

static tree_t *sema_field_lvalue(tree_t *sema, const ctu_t *expr)
{
    tree_t *object = ctu_sema_lvalue(sema, expr->expr);
    const tree_t *ty = get_ptr_type(tree_get_type(object));
    if (!tree_is(ty, eTreeTypeStruct))
    {
        return tree_raise(expr->node, sema->reports, "cannot access field of non-struct type `%s`", tree_to_string(ty));
    }

    tree_t *field = tree_ty_get_field(ty, expr->field);
    if (field == NULL)
    {
        return tree_raise(expr->node, sema->reports, "field `%s` not found in struct `%s`", expr->field, tree_to_string(ty));
    }

    tree_t *ref = tree_type_reference(expr->node, "", tree_get_type(field));
    return tree_expr_field(expr->node, ref, object, field);
}

tree_t *ctu_sema_lvalue(tree_t *sema, const ctu_t *expr)
{
    CTASSERT(expr != NULL);

    switch (expr->kind)
    {
    case eCtuExprName: return sema_name(sema, expr);
    case eCtuExprDeref: return sema_deref_lvalue(sema, expr);
    case eCtuExprIndex: return sema_index_lvalue(sema, expr);
    case eCtuExprField: return sema_field_lvalue(sema, expr);

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

    case eCtuExprName: return sema_load(sema, expr);
    case eCtuExprCall: return sema_call(sema, expr);

    case eCtuExprRef: return sema_ref(sema, expr);
    case eCtuExprDeref: return sema_deref_rvalue(sema, expr);
    case eCtuExprIndex: return sema_index_rvalue(sema, expr);

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

    if (tree_is(actualType, eTreeTypeUnit))
    {
        report(sema->reports, eFatal, stmt->node, "cannot declare a variable of type `unit`");
    }

    const tree_t *inner = ctu_resolve_storage_type(actualType);
    const tree_t *ref = ctu_resolve_decl_type(actualType);
    tree_storage_t storage = {
        .storage = inner,
        .size = ctu_resolve_storage_size(actualType),
        .quals = stmt->mut ? eQualMutable : eQualConst
    };
    tree_t *self = tree_decl_local(decl->node, stmt->name, storage, ref);
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
    const tree_t *type = tree_get_type(decl);

    tree_t *value = stmt->result == NULL ? NULL : util_type_cast(type->result, ctu_sema_rvalue(sema, stmt->result, type->result));
    return tree_stmt_return(stmt->node, value);
}

static tree_t *sema_while(tree_t *sema, tree_t *decl, const ctu_t *stmt)
{
    tree_t *save = ctu_current_loop(sema);

    tree_t *cond = ctu_sema_rvalue(sema, stmt->cond, ctu_get_bool_type());
    tree_t *loop = tree_stmt_loop(stmt->node, cond, tree_stmt_block(stmt->node, vector_of(0)), NULL);

    if (stmt->name != NULL)
    {
        ctu_add_decl(sema, eCtuTagLabels, stmt->name, loop);
    }

    ctu_set_current_loop(sema, loop);

    loop->then = ctu_sema_stmt(sema, decl, stmt->then);
    loop->other = stmt->other == NULL ? NULL : ctu_sema_stmt(sema, decl, stmt->other);

    ctu_set_current_loop(sema, save);

    return loop;
}

static tree_t *sema_assign(tree_t *sema, const ctu_t *stmt)
{
    tree_t *dst = ctu_sema_lvalue(sema, stmt->dst);
    const tree_t *ty = tree_get_type(dst);

    tree_t *src = ctu_sema_rvalue(sema, stmt->src, tree_ty_load_type(ty));

    return tree_stmt_assign(stmt->node, dst, src);
}

static tree_t *get_label_loop(tree_t *sema, const ctu_t *stmt)
{
    if (stmt->label == NULL)
    {
        tree_t *loop = ctu_current_loop(sema);
        if (loop != NULL)
        {
            return loop;
        }

        return tree_raise(stmt->node, sema->reports, "loop control statement not within a loop");
    }

    tree_t *decl = ctu_get_loop(sema, stmt->label);
    if (decl != NULL)
    {
        return decl;
    }

    return tree_raise(stmt->node, sema->reports, "label `%s` not found", stmt->label);
}

static tree_t *sema_break(tree_t *sema, const ctu_t *stmt)
{
    tree_t *loop = get_label_loop(sema, stmt);
    return tree_stmt_jump(stmt->node, loop, eJumpBreak);
}

static tree_t *sema_continue(tree_t *sema, const ctu_t *stmt)
{
    tree_t *loop = get_label_loop(sema, stmt);
    return tree_stmt_jump(stmt->node, loop, eJumpContinue);
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
    case eCtuStmtAssign: return sema_assign(sema, stmt);

    case eCtuStmtBreak: return sema_break(sema, stmt);
    case eCtuStmtContinue: return sema_continue(sema, stmt);

    case eCtuExprCompare:
    case eCtuExprBinary:
    case eCtuExprUnary:
    case eCtuExprName:
        report(sema->reports, eWarn, stmt->node, "expression statement may have no effect");
        /* fallthrough */

    case eCtuExprCall:
        return ctu_sema_rvalue(sema, stmt, NULL);

    default:
        NEVER("invalid stmt kind %d", stmt->kind);
    }
}

size_t ctu_resolve_storage_size(const tree_t *type)
{
    switch (tree_get_kind(type))
    {
    case eTreeTypePointer:
    case eTreeTypeArray:
        CTASSERTF(type->length != SIZE_MAX, "type %s has no length", tree_to_string(type));
        return ctu_resolve_storage_size(type->ptr) * type->length;
    default: return 1;
    }
}

const tree_t *ctu_resolve_storage_type(const tree_t *type)
{
    switch (tree_get_kind(type))
    {
    case eTreeTypeArray: return ctu_resolve_storage_type(type->ptr);
    case eTreeTypePointer: return type->ptr;
    case eTreeTypeReference: NEVER("cannot resolve storage type of reference");

    default: return type;
    }
}

const tree_t *ctu_resolve_decl_type(const tree_t *type)
{
    switch (tree_get_kind(type))
    {
    case eTreeTypeArray:
    case eTreeTypePointer:
        return type;

    case eTreeTypeReference: NEVER("cannot resolve decl type of reference");

    default: return tree_type_reference(tree_get_node(type), tree_get_name(type), type);
    }
}
