#include "sema.h"

#include "ctu/util/report.h"
#include "ctu/util/util.h"

#include "ctu/debug/type.h"
#include "ctu/debug/ast.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * structures
 */

typedef struct sema_t {
    struct sema_t *parent;
    map_t *decls;

    type_t *result; /* return type of current function */
    size_t locals; /* number of locals the current function has */
} sema_t;

/**
 * constants
 */

static type_t *INT_TYPES[INTEGER_END];
static type_t *UINT_TYPES[INTEGER_END];

static sema_t *ROOT_SEMA = NULL;

static type_t *BOOL_TYPE = NULL;
type_t *VOID_TYPE = NULL;

/**
 * builders
 */

static sema_t *base_sema(sema_t *parent, size_t size) {
    sema_t *sema = ctu_malloc(sizeof(sema_t));
    sema->parent = parent;
    sema->decls = new_map(size);
    sema->result = NULL;
    sema->locals = 0;
    return sema;
}

static sema_t *new_sema(sema_t *parent) {
    return base_sema(parent, 32);
}

static void free_sema(sema_t *sema) {
    ctu_free(sema);
}

/**
 * declaration managment
 */

static type_t *return_type(sema_t *sema) {
    type_t *result = sema->result;
    
    if (result) {
        return result;
    }

    if (sema->parent) {
        return return_type(sema->parent);
    }

    return VOID_TYPE;
}

static void add_decl(sema_t *sema, node_t *decl) {
    map_put(sema->decls, get_resolved_name(decl), decl);
}

static node_t *get_decl(sema_t *sema, const char *name) {
    node_t *decl = map_get(sema->decls, name);
    if (decl) {
        mark_used(decl);
        return decl;
    }

    /**
     * recurse if we can
     */
    if (sema->parent) {
        return get_decl(sema->parent, name);
    }

    /**
     * the decl doesnt exist
     */
    return NULL;
}

static void add_decl_unique(sema_t *sema, node_t *node) {
    const char *name = get_resolved_name(node);
    node_t *previous = get_decl(sema, name);

    /**
     * make sure we arent shadowing an existing declaration
     */
    if (previous != NULL) {
        reportf(LEVEL_ERROR, node, "`%s` has already been declared", name);
    }

    add_decl(sema, node);
}

static void add_decl_global(sema_t *sema, node_t *decl) {
    if (is_discard_name(get_decl_name(decl))) {
        reportf(LEVEL_ERROR, decl, "may not discard declaration");
        return;
    }

    add_decl_unique(sema, decl);
}

static void mark_local(node_t *decl) {
    decl->local = ROOT_SEMA->locals++;
}

static void add_local(sema_t *sema, node_t *decl, bool add) {
    if (add) {
        add_decl_unique(sema, decl);
    }

    mark_local(decl);
}

static void add_discardable_local(sema_t *sema, node_t *decl) {
    bool discard = is_discard_name(get_decl_name(decl));
    add_local(sema, decl, !discard);
}

static size_t reset_locals() {
    size_t num = ROOT_SEMA->locals;
    ROOT_SEMA->locals = 0;
    return num;
}

static bool is_local(node_t *node) {
    return node->kind == AST_DECL_VAR 
        || node->kind == AST_DECL_PARAM;
}

static type_t *query_symbol(sema_t *sema, node_t *symbol) {
    const char *name = get_symbol_name(symbol);
    node_t *origin = get_decl(sema, name);

    if (origin == NULL) {
        return new_unresolved(symbol);
    }

    /**
     * if the variable is a local variable
     * then propogate what its index is
     * for the ir gen
     */
    symbol->local = is_local(origin)
        ? origin->local
        : NOT_LOCAL;

    return get_type(origin);
}

static type_t *resolve_symbol(sema_t *sema, node_t *symbol) {
    if (is_discard_name(get_symbol_name(symbol))) {
        reportf(LEVEL_ERROR, symbol, "you cannot resolve the discarded symbol");
        return new_poison(symbol, "discarded symbol");
    }

    type_t *type = query_symbol(sema, symbol);

    if (is_unresolved(type)) {
        reportf(LEVEL_ERROR, symbol, "cannot resolve `%s` to a type", get_symbol_name(symbol));
    }

    connect_type(symbol, type);

    return type;
}

static type_t *resolve_typename(sema_t *sema, node_t *node) {
    type_t *type = raw_type(node);
    if (type) {
        return type;
    }

    return resolve_symbol(sema, node);
}

static type_t *resolve_type(sema_t *sema, node_t *node) {
    if (node->kind == AST_PTR) {
        return new_pointer(node, resolve_type(sema, node->ptr));
    } else if (node->kind == AST_MUT) {
        type_t *type = resolve_type(sema, node->next);
        return set_mut(type, true);
    } else {
        return resolve_typename(sema, node);
    }
}

static node_t *implicit_cast(node_t *original, type_t *to) {
    node_t *cast = ast_cast(original->scanner, original->where, original, NULL);

    connect_type(cast, to);

    return make_implicit(cast);
}

/**
 * comparison
 */

static void check_sign_conversion(node_t **node, type_t *to, type_t *from, bool implicit) {
    if (is_signed(to) != is_signed(from) && implicit) {
        reportf(LEVEL_WARNING, *node, "types do not have the same sign, conversion may be lossy");
        *node = implicit_cast(*node, to);
    }
}

static bool convertible_to(
    node_t **node,
    type_t *to, type_t *from,
    bool implicit
) {
    if (is_void(to)) {
        return is_void(from);
    }
    
    if (is_integer(to) && is_integer(from)) {
        check_sign_conversion(node, to, from, implicit);

        return true;
    }

    if (is_boolean(to)) {
        if (is_boolean(from)) {
            return true;
        }

        if (is_integer(from)) {
            if (implicit) {
                reportid_t report = reportf(LEVEL_WARNING, *node, "implicit integer to boolean conversion");
                report_underline(report, "conversion happens here");
                report_note(report, "add an explicit cast to silence this warning");
                node_t *temp = implicit_cast(*node, to);
                *node = temp;
            }

            return true;
        }
    }

    if (is_pointer(to) && is_pointer(from)) {
        if (to->ptr->mut && !from->ptr->mut) {
            reportf(LEVEL_ERROR, *node, "cannot discard const from pointer type");
        }

        return convertible_to(node, to->ptr, from->ptr, implicit);
    }

    return false;
}

static bool implicit_convertible_to(node_t **node, type_t *to, type_t *from) {
    return convertible_to(node, to, from, true);
}

static bool explicit_convertible_to(type_t *to, type_t *from) {
    return convertible_to(NULL, to, from, false);
}

/**
 * typechecking
 */

static void typecheck_stmt(sema_t *sema, node_t *stmt);
static type_t *typecheck_expr(sema_t *sema, node_t *expr);
static type_t *typecheck_decl(sema_t *sema, node_t *decl);

static type_t *typecheck_return(sema_t *sema, node_t *stmt) {
    node_t *expr = stmt->expr;

    type_t *result = expr == NULL 
        ? VOID_TYPE
        : typecheck_expr(sema, expr);

    if (result == VOID_TYPE && expr != NULL) {
        reportf(LEVEL_ERROR, stmt, "cannot explicitly return void");
    }

    type_t *expect = return_type(sema);

    if (!implicit_convertible_to(&stmt->expr, expect, result)) {
        reportf(LEVEL_ERROR, stmt, "incorrect return type");
    }
    
    result = stmt->expr == NULL
        ? VOID_TYPE
        : get_type(stmt->expr);

    return result;
}

static void typecheck_stmts(sema_t *sema, node_t *stmts) {
    nodes_t *list = get_stmts(stmts);
    
    for (size_t i = 0; i < ast_len(list); i++) {
        typecheck_stmt(sema, ast_at(list, i));
    }
}

static type_t *get_digit_type(node_t *digit) {
    return digit->sign 
        ? INT_TYPES[digit->integer] 
        : UINT_TYPES[digit->integer];
}

static type_t *get_bool_type(void) {
    return BOOL_TYPE;
}

static type_t *typecheck_call(sema_t *sema, node_t *node) {
    type_t *body = typecheck_expr(sema, node->expr);

    if (!is_callable(body)) {
        reportf(LEVEL_ERROR, node, "uncallable type");
        return new_poison(node, "uncallable type");
    }

    size_t params = typelist_len(body->args);
    size_t args = ast_len(node->args);

    if (params != args) {
        reportf(LEVEL_ERROR, node, "incorrect number of parameters");
        return body->result;
    }

    for (size_t i = 0; i < params; i++) {
        node_t *arg = ast_at(node->args, i);
        type_t *to = typelist_at(body->args, i);
        type_t *from = typecheck_expr(sema, arg);
    
        if (!implicit_convertible_to(&arg, to, from)) {
            reportf(LEVEL_ERROR, arg, "incompatible argument type");
        }
    }

    return body->result;
}

static type_t *typecheck_func(sema_t *sema, node_t *decl) {
    type_t *result = resolve_type(sema, decl->result);
    size_t len = ast_len(decl->params);
    types_t *args = new_typelist(len);

    for (size_t i = 0; i < len; i++) {
        node_t *param = ast_at(decl->params, i);
        type_t *arg = typecheck_decl(sema, param);
        typelist_put(args, i, arg);
    }

    return new_callable(decl, args, result);
}

static void check_binary_cmp(node_t* node, type_t *lhs, type_t *rhs) {
    if (!is_integer(lhs) || !is_integer(rhs)) {
        reportf(LEVEL_ERROR, node, "both sides of comparison operation must be integral");
    }
}

static bool check_binary_math(node_t* node, type_t *lhs, type_t *rhs) {
    if (!is_integer(lhs) || !is_integer(rhs)) {
        reportf(LEVEL_ERROR, node, "both sides of math operation must be integral");
        return false;
    }
    return true;
}

static void check_binary_equality(node_t* node, type_t *lhs, type_t *rhs) {
    if (is_integer(lhs) && is_integer(rhs)) {
        return;
    }

    if (is_boolean(lhs) && is_boolean(rhs)) {
        return;
    }

    reportf(LEVEL_ERROR, node, "incompatible comparison operands");
}

// get the largest integer we need
static type_t *get_binary_math_type(type_t *lhs, type_t *rhs) {
    integer_t it = MAX(get_integer_kind(lhs), get_integer_kind(rhs));

    if (is_signed(lhs) || is_signed(rhs)) {
        return INT_TYPES[it];
    } else {
        return UINT_TYPES[it];
    }
}

static type_t *typecheck_binary(sema_t *sema, node_t *expr) {
    type_t *lhs = typecheck_expr(sema, expr->lhs);
    type_t *rhs = typecheck_expr(sema, expr->rhs);
    binary_t op = expr->binary;
    type_t *result = BOOL_TYPE;

    if (is_comparison_op(op)) {
        check_binary_cmp(expr, lhs, rhs);
    } else if (is_equality_op(op)) {
        check_binary_equality(expr, lhs, rhs);
    } else if (is_math_op(op)) {
        if (check_binary_math(expr, lhs, rhs)) {
            result = get_binary_math_type(lhs, rhs);
        }
    } else {
            result = new_poison(expr, "unknown operation");
        }

    return result;
}

static bool is_lvalue(node_t *expr) {
    return expr->kind == AST_SYMBOL;
}

static bool is_assignable(type_t *type, node_t *expr) {
    if (
        expr->kind == AST_UNARY 
        && expr->unary == UNARY_DEREF)
        return is_assignable(type, expr->expr);

    if (expr->kind == AST_SYMBOL)
        return !is_const(type);

    return false;
}

static type_t *typecheck_unary(sema_t *sema, node_t *expr) {
    type_t *type = typecheck_expr(sema, expr->expr);
    unary_t op = expr->unary;

    switch (op) {
    case UNARY_ABS: case UNARY_NEG:
        if (!is_integer(type)) {
            reportf(LEVEL_ERROR, expr, "unary operation requires integral");
        }
        if (!is_signed(type) && op == UNARY_NEG) {
            reportf(LEVEL_WARNING, expr, "unary negation on an unsigned type");
        }
        break;

    case UNARY_REF:
        if (!is_lvalue(expr->expr)) {
            reportf(LEVEL_ERROR, expr, "cannot take a reference to a non-lvalue");
        } else {
            type = new_pointer(expr, type);
        }
        break;
    case UNARY_DEREF:
        if (!is_pointer(type)) {
            reportf(LEVEL_ERROR, expr, "cannot dereference a type that isnt a pointer");
        } else {
            type = type->ptr;
        }
        break;
    case UNARY_TRY:
        reportf(LEVEL_INTERNAL, expr, "unimplemented unary operation");
        break;
    }

    return type;
}

static void typecheck_branch(sema_t *sema, node_t *stmt) {
    if (stmt->cond) {
        type_t *cond = typecheck_expr(sema, stmt->cond);
        if (!implicit_convertible_to(&stmt->cond, BOOL_TYPE, cond)) {
            reportf(LEVEL_ERROR, stmt->cond, "cannot branch on a non-boolean type");
        }
    }

    typecheck_stmt(sema, stmt->branch);

    if (stmt->next) {
        typecheck_branch(sema, stmt->next);
    }
}

static type_t *typecheck_cast(sema_t *sema, node_t *cast) {
    type_t *origin = typecheck_expr(sema, cast->expr);
    type_t *target = resolve_type(sema, cast->cast);

    if (!explicit_convertible_to(target, origin)) {
        reportf(LEVEL_ERROR, cast, "cannot perform explicit conversion");
    }

    return target;
}

static type_t *typecheck_expr(sema_t *sema, node_t *expr) {
    type_t *type = raw_type(expr);

    if (type) {
        return type;
    }

    switch (expr->kind) {
    case AST_DIGIT: 
        type = get_digit_type(expr);
        break;

    case AST_BOOL:
        type = get_bool_type();
        break;

    case AST_BINARY:
        type = typecheck_binary(sema, expr);
        break;

    case AST_UNARY:
        type = typecheck_unary(sema, expr);
        break;

    case AST_SYMBOL:
        type = resolve_symbol(sema, expr);
        break;

    case AST_CALL:
        type = typecheck_call(sema, expr);
        break;

    case AST_CAST:
        type = typecheck_cast(sema, expr);
        break;

    default:
        reportf(LEVEL_INTERNAL, expr, "unimplemented expression typecheck");
        type = new_poison(expr, "unimplemented expression typecheck");
        break;
    }

    connect_type(expr, type);

    return type;
}

static type_t *typecheck_var(sema_t *sema, node_t *decl) {
    type_t *type = NULL;
    type_t *init = NULL;

    type_t *var = NULL;

    if (decl->type) {
        type = resolve_type(sema, decl->type);
        var = type;
    }

    if (decl->init) {
        init = typecheck_expr(sema, decl->init);
        var = init;
    }

    if (type && init) {
        if (!implicit_convertible_to(&decl, type, init)) {
            reportf(LEVEL_ERROR, decl, "variable type and initializer are incompatible");
        }
    }

    var->mut = decl->mut;

    connect_type(decl, var);

    return var;
}

static void typecheck_assign(sema_t *sema, node_t *decl) {
    type_t *dst = typecheck_expr(sema, decl->dst);
    type_t *src = typecheck_expr(sema, decl->src);

    if (!is_assignable(dst, decl->dst)) {
        reportf(LEVEL_ERROR, decl, "cannot assign to a non-lvalue");
    }

    if (!implicit_convertible_to(&decl, dst, src)) {
        reportf(LEVEL_ERROR, decl, "cannot assign unrelated types");
    }
}

static void typecheck_while(sema_t *sema, node_t *decl) {
    type_t *cond = typecheck_expr(sema, decl->cond);
    typecheck_stmt(sema, decl->next);

    if (!implicit_convertible_to(&decl->cond, BOOL_TYPE, cond)) {
        reportf(LEVEL_ERROR, decl->cond, "cannot loop on a non-boolean condition");
    }
}

static type_t *typecheck_decl(sema_t *sema, node_t *decl) {
    type_t *type = raw_type(decl);

    if (type) {
        return type;
    }

    switch (decl->kind) {
    case AST_DECL_PARAM:
        type = resolve_type(sema, decl->type);
        break;

    case AST_DECL_FUNC:
        type = typecheck_func(sema, decl);
        break;

    case AST_DECL_VAR:
        type = typecheck_var(sema, decl);
        add_discardable_local(sema, decl);
        return type;

    default:
        reportf(LEVEL_INTERNAL, decl, "unimplemented declaration typecheck");
        type = new_poison(decl, "unimplemented declaration typecheck");
        break;
    }

    connect_type(decl, type);

    return type;
}

static void typecheck_stmt(sema_t *sema, node_t *stmt) {
    type_t *type = VOID_TYPE;
    sema_t *nest;

    switch (stmt->kind) {
    case AST_RETURN:
        type = typecheck_return(sema, stmt);
        break;

    case AST_STMTS:
        nest = base_sema(sema, 8);
        typecheck_stmts(nest, stmt);
        free_sema(nest);
        break;

    case AST_BRANCH:
        typecheck_branch(sema, stmt);
        break;

    case AST_DECL_VAR:
        type = typecheck_decl(sema, stmt);
        break;

    case AST_DIGIT: case AST_UNARY: 
    case AST_BINARY: case AST_CALL:
        type = typecheck_expr(sema, stmt);
        if (!is_void(type)) {
            reportf(LEVEL_WARNING, stmt, "discarding value of expression");
        }
        break;

    case AST_ASSIGN:
        typecheck_assign(sema, stmt);
        break;

    case AST_WHILE:
        typecheck_while(sema, stmt);
        break;

    default:
        reportf(LEVEL_INTERNAL, stmt, "unimplement statement typecheck");
        break;
    }

    connect_type(stmt, type);
}

static void validate_params(sema_t *sema, nodes_t *params) {
    size_t len = ast_len(params);
    for (size_t i = 0; i < len; i++) {
        node_t *param = ast_at(params, i);
        type_t *type = typecheck_decl(sema, param);
        
        add_discardable_local(sema, param);

        if (is_void(type)) {
            reportf(LEVEL_ERROR, param, "parameter cannot have void type");
        }
    }
}

static void validate_function(sema_t *sema, node_t *func) {
    sema_t *nest = new_sema(sema);
    validate_params(nest, func->params);
    sema->result = resolve_type(sema, func->result);

    typecheck_stmts(nest, func->body);

    free_sema(nest);
}

static void add_all_decls(sema_t *sema, nodes_t *decls) {
    size_t len = ast_len(decls);

    for (size_t i = 0; i < len; i++) {
        node_t *decl = ast_at(decls, i);
        if (decl->kind == AST_DECL_FUNC)
            typecheck_func(sema, decl);
        else
            typecheck_var(sema, decl);
        add_decl_global(sema, decl);
    }
}

static void validate_var(sema_t *sema, node_t *var) {
    typecheck_decl(sema, var);
}

static void typecheck_all_decls(sema_t *sema, nodes_t *decls) {
    add_all_decls(sema, decls);

    size_t len = ast_len(decls);

    for (size_t i = 0; i < len; i++) {
        node_t *decl = ast_at(decls, i);
        if (decl->kind == AST_DECL_VAR) {
            validate_var(sema, decl);
        } else {
            validate_function(sema, decl);
            decl->locals = reset_locals();
        }
    }
}

static void add_builtin(type_t *type) {
    add_decl(ROOT_SEMA, type->node);
}

/**
 * external api
 */

void typecheck(nodes_t *nodes) {
    sema_t *sema = base_sema(ROOT_SEMA, 256);
    typecheck_all_decls(sema, nodes);
    free_sema(sema);
}

static void add_int(int kind, const char *name) {
    INT_TYPES[kind] = new_integer(kind, true, name);
}

static void add_uint(int kind, const char *name) {
    UINT_TYPES[kind] = new_integer(kind, false, name);
}

void sema_init(void) {
    ROOT_SEMA = new_sema(NULL);

    add_int(INTEGER_CHAR, "char");
    add_int(INTEGER_SHORT, "short");
    add_int(INTEGER_INT, "int");
    add_int(INTEGER_LONG, "long");
    add_int(INTEGER_SIZE, "isize");
    add_int(INTEGER_INTPTR, "intptr");
    add_int(INTEGER_INTMAX, "intmax");
    
    add_uint(INTEGER_CHAR, "uchar");
    add_uint(INTEGER_SHORT, "ushort");
    add_uint(INTEGER_INT, "uint");
    add_uint(INTEGER_LONG, "ulong");
    add_uint(INTEGER_SIZE, "usize");
    add_uint(INTEGER_INTPTR, "uintptr");
    add_uint(INTEGER_INTMAX, "uintmax");
    
    BOOL_TYPE = new_builtin(TYPE_BOOLEAN, "bool");
    VOID_TYPE = new_builtin(TYPE_VOID, "void");

    for (int i = 0; i < INTEGER_END; i++) {
        add_builtin(INT_TYPES[i]);
        add_builtin(UINT_TYPES[i]);
    }

    add_builtin(BOOL_TYPE);
    add_builtin(VOID_TYPE);
}
