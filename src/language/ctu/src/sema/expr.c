// SPDX-License-Identifier: GPL-3.0-only

#include "ctu/sema/expr.h"
#include "cthulhu/events/events.h"
#include "ctu/sema/decl/resolve.h"
#include "ctu/sema/type.h"
#include "ctu/driver.h"

#include "cthulhu/util/types.h"
#include "cthulhu/util/util.h"

#include "cthulhu/tree/query.h"

#include "ctu/ast.h"

#include "memory/memory.h"
#include "std/str.h"
#include "std/vector.h"

#include "base/panic.h"

#include "core/macros.h"

///
/// get decls
///

static const size_t kModuleTags[] = {eCtuTagModules, eCtuTagImports, eCtuTagTypes};
static const size_t kDeclTags[] = {eCtuTagValues, eCtuTagFunctions};

static const decl_search_t kSearchName = {
    .module_tags = kModuleTags,
    .module_count = sizeof(kModuleTags) / sizeof(size_t),

    .decl_tags = kDeclTags,
    .decl_count = sizeof(kDeclTags) / sizeof(size_t),
};

static bool is_public(const tree_t *decl)
{
    const tree_attribs_t *attrib = tree_get_attrib(decl);
    return attrib->visibility == eVisiblePublic;
}

// TODO: needsLoad is awful
static tree_t *sema_decl_name(tree_t *sema, const node_t *node, const vector_t *path, bool *needs_load)
{
    bool is_imported = false;
    tree_t *ns = util_search_namespace(sema, &kSearchName, node, path, &is_imported);
    if (tree_is(ns, eTreeError))
    {
        return ns;
    }

    const char *name = vector_tail(path);
    if (tree_is(ns, eTreeTypeEnum))
    {
        tree_t *resolve = tree_resolve(tree_get_cookie(sema), ns);
        const tree_t *it = tree_ty_get_case(resolve, name);
        if (it != NULL)
        {
            *needs_load = false;
            return it->case_value;
        }

        return tree_raise(node, sema->reports, &kEvent_SymbolNotFound,
                          "enum case `%s` not found in `%s`", name, tree_to_string(ns));
    }
    else if (tree_is(ns, eTreeDeclModule))
    {
        tree_t *decl = util_select_decl(ns, kDeclTags, sizeof(kDeclTags) / sizeof(size_t), name);
        if (decl == NULL)
        {
            return tree_raise(node, sema->reports, &kEvent_SymbolNotFound,
                              "declaration `%s` not found in `%s`", name, tree_to_string(ns));
        }

        if (is_imported && !is_public(decl))
        {
            msg_notify(sema->reports, &kEvent_SymbolNotVisible, node,
                       "cannot access non-public declaration `%s`", name);
        }

        if (tree_is(decl, eTreeDeclFunction) || tree_is(decl, eTreeDeclParam))
        {
            *needs_load = false;
            return decl;
        }

        return tree_resolve(tree_get_cookie(sema), decl);
    }

    CT_NEVER("invalid namespace type %s", tree_to_string(ns));
}

///
/// inner logic
///

static tree_t *verify_expr_type(tree_t *sema, tree_kind_t kind, const tree_t *type,
                                const char *expr_kind, tree_t *expr)
{
    CT_UNUSED(sema);
    CT_UNUSED(kind);
    CT_UNUSED(expr_kind);

    if (type == NULL)
    {
        return expr;
    }

    tree_t *result = util_type_cast(type, expr);
    if (tree_is(result, eTreeError))
    {
        tree_report(sema->reports, result);
    }
    return result;
}

static tree_t *sema_bool(tree_t *sema, const ctu_t *expr, const tree_t *implicit_type)
{
    const tree_t *type = implicit_type ? implicit_type : ctu_get_bool_type();
    if (!tree_is(type, eTreeTypeBool))
    {
        return tree_raise(expr->node, sema->reports, &kEvent_InvalidLiteralType, "invalid type `%s` for boolean literal",
                          tree_to_string(type));
    }

    tree_t *it = tree_expr_bool(expr->node, type, expr->bool_value);

    return verify_expr_type(sema, eTreeTypeBool, type, "boolean literal", it);
}

static tree_t *sema_int(tree_t *sema, const ctu_t *expr, const tree_t *implicit_type)
{
    const tree_t *type = implicit_type
        ? implicit_type
        : ctu_get_int_type(eDigitInt, eSignSigned); // TODO: calculate proper type to use
    if (!tree_is(type, eTreeTypeDigit))
    {
        return tree_raise(expr->node, sema->reports, &kEvent_InvalidLiteralType, "invalid type `%s` for integer literal",
                          tree_to_string(type));
    }

    tree_t *it = tree_expr_digit(expr->node, type, expr->int_value);

    return verify_expr_type(sema, eTreeTypeDigit, type, "integer literal", it);
}

static tree_t *sema_cast(ctu_sema_t *sema, const ctu_t *expr)
{
    const tree_t *ty = ctu_sema_type(sema, expr->cast);
    tree_t *inner = ctu_sema_rvalue(sema, expr->expr, NULL);
    tree_t *cast = util_type_cast(ty, inner);
    if (tree_is(cast, eTreeError))
    {
        tree_report(ctu_sema_reports(sema), cast);
    }
    return cast;
}

static tree_t *sema_string(tree_t *sema, const ctu_t *expr, const tree_t *implicit_type)
{
    CT_UNUSED(sema);

    // +1 length for the nul terminator
    tree_t *type = tree_type_array(expr->node, "str", ctu_get_char_type(), expr->length + 1);
    tree_t *str = tree_expr_string(expr->node, type, expr->text, expr->length);
    if (implicit_type != NULL)
    {
        return ctu_cast_type(sema, str, implicit_type);
    }

    return str;
}

static tree_t *sema_name(tree_t *sema, const ctu_t *expr)
{
    bool needs_load = false;
    return sema_decl_name(sema, expr->node, expr->path, &needs_load);
}

static tree_t *sema_load(tree_t *sema, const ctu_t *expr, const tree_t *implicit_type)
{
    bool needs_load = true;

    tree_t *name = sema_decl_name(sema, expr->node, expr->path, &needs_load);

    if (needs_load)
    {
        name = tree_expr_load(expr->node, name);
    }

    if (implicit_type != NULL)
    {
        name = tree_resolve_type(name);
        return ctu_cast_type(sema, name, implicit_type);
    }

    return name;
}

static tree_t *sema_compare(ctu_sema_t *sema, const ctu_t *expr)
{
    tree_t *left = ctu_sema_rvalue(sema, expr->lhs, NULL);
    tree_t *right = ctu_sema_rvalue(sema, expr->rhs, NULL);

    if (!util_types_comparable(tree_get_type(left), tree_get_type(right)))
    {
        return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_InvalidBinaryOperation, "cannot compare `%s` to `%s`",
                          tree_to_string(tree_get_type(left)),
                          tree_to_string(tree_get_type(right)));
    }

    return tree_expr_compare(expr->node, ctu_get_bool_type(), expr->compare, left, right);
}

static tree_t *sema_binary(ctu_sema_t *sema, const ctu_t *expr, const tree_t *implicit_type)
{
    tree_t *left = ctu_sema_rvalue(sema, expr->lhs, implicit_type);
    tree_t *right = ctu_sema_rvalue(sema, expr->rhs, implicit_type);

    if (tree_is(left, eTreeError) || tree_is(right, eTreeError))
    {
        return tree_error(expr->node, &kEvent_InvalidBinaryOperation, "invalid binary");
    }

    // TODO: calculate proper type to use
    const tree_t *common_type = implicit_type == NULL ? tree_get_type(left) : implicit_type;

    return tree_expr_binary(expr->node, common_type, expr->binary, left, right);
}

static tree_t *sema_unary(ctu_sema_t *sema, const ctu_t *expr, const tree_t *implicit_type)
{
    tree_t *inner = ctu_sema_rvalue(sema, expr->expr, implicit_type);

    if (tree_is(inner, eTreeError))
    {
        return tree_error(expr->node, &kEvent_InvalidUnaryOperation, "invalid unary");
    }

    switch (expr->unary)
    {
    case eUnaryAbs:
    case eUnaryNeg:
    case eUnaryFlip:
        if (!tree_is(tree_get_type(inner), eTreeTypeDigit))
        {
            return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_InvalidUnaryOperation,
                              "cannot apply unary `%s` to non-digit type `%s`",
                              unary_name(expr->unary), tree_to_string(tree_get_type(inner)));
        }
        break;

    case eUnaryNot:
        if (!tree_is(tree_get_type(inner), eTreeTypeBool))
        {
            return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_InvalidUnaryOperation,
                              "cannot apply unary `%s` to non-bool type `%s`",
                              unary_name(expr->unary), tree_to_string(tree_get_type(inner)));
        }
        break;

    default: CT_NEVER("invalid unary %d", expr->unary);
    }

    return tree_expr_unary(expr->node, expr->unary, inner);
}

static const tree_t *get_param_checked(const vector_t *params, size_t i)
{
    if (i >= vector_len(params))
    {
        return NULL;
    }

    const tree_t *param = vector_get(params, i);
    return tree_get_type(param);
}

static tree_t *sema_call(ctu_sema_t *sema, const ctu_t *expr)
{
    tree_t *callee = ctu_sema_lvalue(sema, expr->callee);
    if (tree_is(callee, eTreeError))
    {
        return callee;
    }

    const tree_t *type = tree_get_type(tree_resolve_type(callee));
    if (tree_is(type, eTreeTypeReference))
    {
        callee = tree_expr_load(expr->node, callee);
    }

    const vector_t *params = tree_fn_get_params(type);

    arena_t *arena = get_global_arena();
    size_t len = vector_len(expr->args);
    vector_t *result = vector_of(len, arena);
    for (size_t i = 0; i < len; i++)
    {
        const tree_t *ty = get_param_checked(params, i);

        ctu_t *it = vector_get(expr->args, i);
        tree_t *arg = ctu_sema_rvalue(sema, it, ty);
        vector_set(result, i, arg);
    }

    return tree_expr_call(expr->node, callee, result);
}

static const tree_t *get_ptr_type(const tree_t *ty)
{
    if (tree_is(ty, eTreeTypeReference))
    {
        return ty->ptr;
    }

    return ty;
}

static tree_t *sema_deref_lvalue(ctu_sema_t *sema, const ctu_t *expr)
{
    tree_t *inner = ctu_sema_rvalue(sema, expr->expr, NULL);
    if (tree_is(inner, eTreeError))
    {
        return inner;
    }

    mpz_t zero;
    mpz_init_set_ui(zero, 0);
    tree_t *index = tree_expr_digit(expr->node, ctu_get_int_type(eDigitSize, eSignUnsigned), zero);

    const tree_t *type = tree_get_type(inner);
    const tree_t *ty = get_ptr_type(type);
    if (!tree_is(ty, eTreeTypePointer))
    {
        return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_InvalidDereference,
                          "cannot dereference non-pointer type `%s` inside lvalue", tree_to_string(ty));
    }

    tree_t *ref = tree_type_reference(expr->node, tree_get_name(type), ty->ptr);
    return tree_expr_offset(expr->node, ref, inner, index);
}

static tree_t *sema_deref_rvalue(ctu_sema_t *sema, const ctu_t *expr)
{
    tree_t *inner = ctu_sema_rvalue(sema, expr->expr, NULL);
    if (tree_is(inner, eTreeError))
    {
        return inner;
    }

    return tree_expr_load(expr->node, inner);
}

static tree_t *sema_ref(ctu_sema_t *sema, const ctu_t *expr)
{
    tree_t *inner = ctu_sema_lvalue(sema, expr->expr);
    if (tree_is(inner, eTreeError))
    {
        return inner;
    }

    if (tree_is(inner, eTreeDeclLocal))
    {
        // TODO: do we need a way to get a pointer to a local?
        const tree_t *type = tree_get_storage_type(inner);
        const tree_t *ptr = tree_type_pointer(expr->node, "", type, 1);
        return tree_expr_cast(expr->node, ptr, inner);
    }

    return tree_expr_address(expr->node, inner);
}

static bool can_index_type(const tree_t *ty)
{
    switch (tree_get_kind(ty))
    {
    case eTreeTypePointer:
    case eTreeTypeArray: return true;

    default: return false;
    }
}

static tree_t *sema_index_rvalue(ctu_sema_t *sema, const ctu_t *expr)
{
    tree_t *index = ctu_sema_rvalue(sema, expr->index, ctu_get_int_type(eDigitSize, eSignUnsigned));
    tree_t *object = ctu_sema_lvalue(sema, expr->expr);

    const tree_t *ty = get_ptr_type(tree_get_type(object));
    if (!can_index_type(ty))
    {
        return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_InvalidIndex,
                          "cannot index non-pointer type `%s` inside rvalue", tree_to_string(ty));
    }

    tree_t *offset = tree_expr_offset(expr->node, ty, object, index);
    return tree_expr_load(expr->node, offset);
}

static tree_t *sema_index_lvalue(ctu_sema_t *sema, const ctu_t *expr)
{
    tree_t *index = ctu_sema_rvalue(sema, expr->index, ctu_get_int_type(eDigitSize, eSignUnsigned));
    tree_t *object = ctu_sema_lvalue(sema, expr->expr);

    const tree_t *ty = get_ptr_type(tree_get_type(object));
    if (!can_index_type(ty))
    {
        return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_InvalidIndex,
                          "cannot index non-pointer type `%s` inside lvalue", tree_to_string(ty));
    }

    tree_t *ref = tree_type_reference(expr->node, "", ty->ptr);
    return tree_expr_offset(expr->node, ref, object, index);
}

///
/// fields
/// TODO: so much duplicated logic
///

/// TODO: what the fuck

static tree_t *sema_field_lvalue(ctu_sema_t *sema, const ctu_t *expr)
{
    tree_t *object = ctu_sema_lvalue(sema, expr->expr);
    const tree_t *ty = get_ptr_type(tree_get_type(object));
    if (!tree_is(ty, eTreeTypeStruct))
    {
        return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_InvalidIndirection,
                          "cannot access field of non-struct type `%s`", tree_to_string(ty));
    }

    tree_t *field = tree_ty_get_field(ty, expr->field);
    if (field == NULL)
    {
        return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_SymbolNotFound,
                          "field `%s` not found in struct `%s`", expr->field, tree_to_string(ty));
    }

    tree_t *ref = tree_type_reference(expr->node, "", tree_get_type(field));
    return tree_expr_field(expr->node, ref, object, field);
}

static tree_t *sema_field_rvalue(ctu_sema_t *sema, const ctu_t *expr)
{
    tree_t *object = ctu_sema_lvalue(sema, expr->expr);
    const tree_t *ty = get_ptr_type(tree_get_type(object));
    if (!tree_is(ty, eTreeTypeStruct))
    {
        return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_InvalidIndirection,
                          "cannot access field of non-struct type `%s`", tree_to_string(ty));
    }

    tree_t *field = tree_ty_get_field(ty, expr->field);
    if (field == NULL)
    {
        return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_SymbolNotFound,
                          "field `%s` not found in struct `%s`", expr->field, tree_to_string(ty));
    }

    tree_t *ref = tree_type_reference(expr->node, "", tree_get_type(field));
    tree_t *access = tree_expr_field(expr->node, ref, object, field);
    return tree_expr_load(expr->node, access);
}

static tree_t *sema_field_indirect_lvalue(ctu_sema_t *sema, const ctu_t *expr)
{
    tree_t *object = ctu_sema_lvalue(sema, expr->expr);
    const tree_t *ptr = get_ptr_type(tree_get_type(object));
    if (!tree_is(ptr, eTreeTypePointer) || !tree_is(ptr->ptr, eTreeTypeStruct))
    {
        return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_InvalidIndirection,
                          "cannot indirectly access field of non-pointer-to-struct type `%s`",
                          tree_to_string(ptr));
    }

    const tree_t *ty = ptr->ptr;
    tree_t *field = tree_ty_get_field(ty, expr->field);
    if (field == NULL)
    {
        return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_FieldNotFound,
                          "field `%s` not found in struct `%s`", expr->field, tree_to_string(ty));
    }

    tree_t *ref = tree_type_reference(expr->node, "", tree_get_type(field));
    return tree_expr_field(expr->node, ref, object, field);
}

static tree_t *sema_field_indirect_rvalue(ctu_sema_t *sema, const ctu_t *expr)
{
    tree_t *object = ctu_sema_lvalue(sema, expr->expr);
    const tree_t *ptr = get_ptr_type(tree_get_type(object));
    if (!tree_is(ptr, eTreeTypePointer) || !tree_is(ptr->ptr, eTreeTypeStruct))
    {
        return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_InvalidIndirection,
                          "cannot indirectly access field of non-pointer-to-struct type `%s`",
                          tree_to_string(ptr));
    }

    const tree_t *ty = ptr->ptr;
    tree_t *field = tree_ty_get_field(ty, expr->field);
    if (field == NULL)
    {
        return tree_raise(expr->node, ctu_sema_reports(sema), &kEvent_FieldNotFound,
                          "field `%s` not found in struct `%s`", expr->field, tree_to_string(ty));
    }

    tree_t *ref = tree_type_reference(expr->node, "", tree_get_type(field));
    tree_t *access = tree_expr_field(expr->node, ref, object, field);
    return tree_expr_load(expr->node, access);
}

static tree_t *sema_init(ctu_sema_t *sema, const ctu_t *expr, const tree_t *implicit_type)
{
    logger_t *reports = ctu_sema_reports(sema);
    if (implicit_type == NULL)
    {
        return tree_raise(expr->node, reports, &kEvent_InvalidInitializer, "cannot infer type of initializer");
    }

    if (!tree_is(implicit_type, eTreeTypeStruct))
    {
        return tree_raise(expr->node, reports, &kEvent_InvalidInitializer, "cannot initialize non-struct type `%s`",
                          tree_to_string(implicit_type));
    }

    const tree_t *ref = ctu_resolve_decl_type(implicit_type);

    tree_storage_t storage = {.storage = implicit_type, .length = 1, .quals = eQualMutable};
    tree_t *local = tree_decl_local(expr->node, "$tmp", storage, ref);
    tree_add_local(sema->decl, local);

    size_t len = vector_len(expr->inits);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *init = vector_get(expr->inits, i);
        CTASSERTF(init->kind == eCtuFieldInit, "invalid init kind %d", init->kind);

        tree_t *field = tree_ty_get_field(implicit_type, init->field);
        if (field == NULL)
        {
            msg_notify(reports, &kEvent_FieldNotFound, init->node,
                       "field `%s` not found in struct `%s`", init->field,
                       tree_to_string(implicit_type));
            continue;
        }

        tree_t *value = ctu_sema_rvalue(sema, init->expr, tree_get_type(field));
        tree_t *ref_type = tree_type_reference(init->node, "", tree_get_type(field));
        tree_t *dst = tree_expr_field(init->node, ref_type, local, field);
        tree_t *assign = tree_stmt_assign(init->node, dst, value);

        vector_push(&sema->block, assign);
    }

    // TODO: default init remaining fields

    return tree_expr_load(expr->node, local);
}

tree_t *ctu_sema_lvalue(ctu_sema_t *sema, const ctu_t *expr)
{
    CTASSERT(expr != NULL);

    switch (expr->kind)
    {
    case eCtuExprName: return sema_name(sema->sema, expr);
    case eCtuExprDeref: return sema_deref_lvalue(sema, expr);
    case eCtuExprIndex: return sema_index_lvalue(sema, expr);
    case eCtuExprField: return sema_field_lvalue(sema, expr);
    case eCtuExprFieldIndirect: return sema_field_indirect_lvalue(sema, expr);

    default: CT_NEVER("invalid lvalue-expr kind %d", expr->kind);
    }
}

tree_t *ctu_sema_rvalue(ctu_sema_t *sema, const ctu_t *expr, const tree_t *implicit_type)
{
    CTASSERT(expr != NULL);

    const tree_t *inner = implicit_type == NULL
        ? NULL
        : tree_resolve(tree_get_cookie(sema->sema), implicit_type);

    switch (expr->kind)
    {
    case eCtuExprBool: return sema_bool(sema->sema, expr, inner);
    case eCtuExprInt: return sema_int(sema->sema, expr, inner);
    case eCtuExprString: return sema_string(sema->sema, expr, inner);
    case eCtuExprCast: return sema_cast(sema, expr);
    case eCtuExprInit: return sema_init(sema, expr, inner);

    case eCtuExprName: return sema_load(sema->sema, expr, inner);
    case eCtuExprCall: return sema_call(sema, expr);

    case eCtuExprRef: return sema_ref(sema, expr);
    case eCtuExprDeref: return sema_deref_rvalue(sema, expr);
    case eCtuExprIndex: return sema_index_rvalue(sema, expr);
    case eCtuExprField: return sema_field_rvalue(sema, expr);
    case eCtuExprFieldIndirect: return sema_field_indirect_rvalue(sema, expr);

    case eCtuExprCompare: return sema_compare(sema, expr);
    case eCtuExprBinary: return sema_binary(sema, expr, inner);
    case eCtuExprUnary: return sema_unary(sema, expr, inner);

    default: CT_NEVER("invalid rvalue-expr kind %d", expr->kind);
    }
}

static tree_t *sema_local(ctu_sema_t *sema, const ctu_t *stmt)
{
    tree_t *type = stmt->type == NULL ? NULL : ctu_sema_type(sema, stmt->type);
    tree_t *value = stmt->value == NULL ? NULL : ctu_sema_rvalue(sema, stmt->value, type);

    CTASSERT(value != NULL || type != NULL);

    const tree_t *actual_type = type != NULL
        ? tree_resolve(tree_get_cookie(sema->sema), type)
        : tree_get_type(value);

    if (tree_is(actual_type, eTreeTypeUnit))
    {
        msg_notify(ctu_sema_reports(sema), &kEvent_InvalidVariableType, stmt->node,
                   "cannot declare a variable of type `unit`");
    }

    const tree_t *ref = tree_type_reference(stmt->node, stmt->name, actual_type);
    tree_storage_t storage = {
        .storage = actual_type,
        .length = 1,
        .quals = stmt->mut ? eQualMutable : eQualConst,
    };
    tree_t *self = tree_decl_local(stmt->node, stmt->name, storage, ref);
    tree_add_local(sema->decl, self);
    ctu_add_decl(sema->sema, eCtuTagValues, stmt->name, self);

    if (value != NULL)
    {
        return tree_stmt_assign(stmt->node, self, value);
    }

    // TODO: maybe a nop node would be better
    return tree_stmt_block(stmt->node, &kEmptyVector);
}

static tree_t *sema_stmts(ctu_sema_t *sema, const ctu_t *stmt)
{
    tree_t *decl = sema->decl;
    size_t len = vector_len(stmt->stmts);

    size_t sizes[eCtuTagTotal] = {[eCtuTagTypes] = 4, [eCtuTagValues] = 4, [eCtuTagFunctions] = 4,};

    tree_t *ctx = tree_module(sema->sema, stmt->node, decl->name, eCtuTagTotal, sizes);
    arena_t *arena = get_global_arena();
    ctu_sema_t inner = ctu_sema_nested(sema, ctx, sema->decl, vector_new(len, arena));
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *it = vector_get(stmt->stmts, i);
        tree_t *step = ctu_sema_stmt(&inner, it);
        vector_push(&inner.block, step);
    }

    return tree_stmt_block(stmt->node, inner.block);
}

static tree_t *sema_return(ctu_sema_t *sema, const ctu_t *stmt)
{
    const tree_t *result = tree_fn_get_return(sema->decl);

    if (stmt->result == NULL)
    {
        return tree_stmt_return(stmt->node, tree_expr_unit(stmt->node, result));
    }

    tree_t *value = ctu_sema_rvalue(sema, stmt->result, result);

    return tree_stmt_return(stmt->node, value);
}

static tree_t *sema_while(ctu_sema_t *sema, const ctu_t *stmt)
{
    tree_t *save = ctu_current_loop(sema);

    tree_t *cond = ctu_sema_rvalue(sema, stmt->cond, ctu_get_bool_type());
    tree_t *loop = tree_stmt_loop(stmt->node, cond, tree_stmt_block(stmt->node, &kEmptyVector),
                                  NULL);

    if (stmt->name != NULL)
    {
        ctu_add_decl(sema->sema, eCtuTagLabels, stmt->name, loop);
    }

    ctu_set_current_loop(sema, loop);

    loop->then = ctu_sema_stmt(sema, stmt->then);
    ctu_set_current_loop(sema, save);

    loop->other = stmt->other == NULL ? NULL : ctu_sema_stmt(sema, stmt->other);

    return loop;
}

static tree_t *sema_assign(ctu_sema_t *sema, const ctu_t *stmt)
{
    tree_t *dst = ctu_sema_lvalue(sema, stmt->dst);
    const tree_t *ty = tree_get_type(dst);

    tree_t *src = ctu_sema_rvalue(sema, stmt->src, tree_ty_load_type(ty));

    return tree_stmt_assign(stmt->node, dst, src);
}

static tree_t *sema_branch(ctu_sema_t *sema, const ctu_t *stmt)
{
    tree_t *cond = ctu_sema_rvalue(sema, stmt->cond, ctu_get_bool_type());
    tree_t *then = ctu_sema_stmt(sema, stmt->then);
    tree_t *other = stmt->other == NULL ? NULL : ctu_sema_stmt(sema, stmt->other);

    return tree_stmt_branch(stmt->node, cond, then, other);
}

static tree_t *get_label_loop(ctu_sema_t *sema, const ctu_t *stmt)
{
    tree_t *ctx = sema->sema;
    if (stmt->label == NULL)
    {
        tree_t *loop = ctu_current_loop(sema);
        if (loop != NULL)
        {
            return loop;
        }

        return tree_raise(stmt->node, ctx->reports, &kEvent_InvalidControlFlow, "loop control statement not within a loop");
    }

    tree_t *decl = ctu_get_loop(ctx, stmt->label);
    if (decl != NULL)
    {
        return decl;
    }

    return tree_raise(stmt->node, ctx->reports, &kEvent_SymbolNotFound, "label `%s` not found",
                      stmt->label);
}

static tree_t *sema_break(ctu_sema_t *sema, const ctu_t *stmt)
{
    tree_t *loop = get_label_loop(sema, stmt);
    return tree_stmt_jump(stmt->node, loop, eJumpBreak);
}

static tree_t *sema_continue(ctu_sema_t *sema, const ctu_t *stmt)
{
    tree_t *loop = get_label_loop(sema, stmt);
    return tree_stmt_jump(stmt->node, loop, eJumpContinue);
}

tree_t *ctu_sema_stmt(ctu_sema_t *sema, const ctu_t *stmt)
{
    CTASSERT(sema->sema != NULL);
    CTASSERT(sema->decl != NULL);
    CTASSERT(sema->block != NULL);

    CTASSERT(stmt != NULL);

    switch (stmt->kind)
    {
    case eCtuStmtLocal: return sema_local(sema, stmt);
    case eCtuStmtList: return sema_stmts(sema, stmt);
    case eCtuStmtReturn: return sema_return(sema, stmt);
    case eCtuStmtWhile: return sema_while(sema, stmt);
    case eCtuStmtAssign: return sema_assign(sema, stmt);
    case eCtuStmtBranch: return sema_branch(sema, stmt);

    case eCtuStmtBreak: return sema_break(sema, stmt);
    case eCtuStmtContinue: return sema_continue(sema, stmt);

    case eCtuExprCompare:
    case eCtuExprBinary:
    case eCtuExprUnary:
    case eCtuExprName:
        msg_notify(ctu_sema_reports(sema), &kEvent_ExpressionHasNoEffect, stmt->node,
                   "expression statement may have no effect");
        /* fallthrough */

    case eCtuExprCall: return ctu_sema_rvalue(sema, stmt, NULL);

    default: CT_NEVER("invalid stmt kind %d", stmt->kind);
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
    case eTreeTypeReference: CT_NEVER("cannot resolve storage type of reference");

    default: return type;
    }
}

const tree_t *ctu_resolve_decl_type(const tree_t *type)
{
    switch (tree_get_kind(type))
    {
    case eTreeTypeArray:
    case eTreeTypePointer: return type;

    case eTreeTypeReference: CT_NEVER("cannot resolve decl type of reference");

    default: return tree_type_reference(tree_get_node(type), tree_get_name(type), type);
    }
}
