#include "sema.h"

#include "ctu/util/report.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * structures
 */

typedef struct sema_t {
    struct sema_t *parent;
    nodes_t *decls;

    type_t *result;
} sema_t;

/**
 * constants
 */

static sema_t *ROOT_SEMA = NULL;

static type_t *INT_TYPE = NULL;
static type_t *UINT_TYPE = NULL;
static type_t *BOOL_TYPE = NULL;
static type_t *VOID_TYPE = NULL;

/**
 * builders
 */

static sema_t *new_sema(sema_t *parent) {
    sema_t *sema = malloc(sizeof(sema_t));
    sema->parent = parent;
    sema->decls = ast_list(NULL);
    sema->result = NULL;
    return sema;
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
    ast_append(sema->decls, decl);
}

static node_t *get_decl(sema_t *sema, const char *name) {
    for (size_t i = 0; i < ast_len(sema->decls); i++) {
        node_t *other = ast_at(sema->decls, i);
        const char *id = get_resolved_name(other);

        if (strcmp(id, name) == 0) {
            return other;
        }
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

static type_t *query_symbol(sema_t *sema, node_t *symbol) {
    const char *name = get_symbol_name(symbol);
    node_t *origin = get_decl(sema, name);

    if (origin == NULL) {
        return new_unresolved(symbol);
    }

    return get_type(origin);
}

static type_t *resolve_symbol(sema_t *sema, node_t *symbol) {
    type_t *type = query_symbol(sema, symbol);

    if (is_unresolved(type)) {
        reportf(LEVEL_ERROR, symbol, "cannot resolve `%s` to a type", get_symbol_name(symbol));
    }

    connect_type(symbol, type);

    return type;
}

/**
 * comparison
 */

static bool convertible_to(type_t *to, type_t *from) {
    if (is_void(to)) {
        return is_void(from);
    }
    
    if (is_integer(to)) {
        return is_integer(from);
    }

    if (is_boolean(to)) {
        return is_integer(from) || is_boolean(from);
    }

    return false;
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

    if (!convertible_to(expect, result)) {
        reportf(LEVEL_ERROR, stmt, "incorrect return type");
    }

    return result;
}

static void typecheck_stmts(sema_t *sema, node_t *stmts) {
    sema_t *nest = new_sema(sema);

    nodes_t *list = get_stmts(stmts);
    
    for (size_t i = 0; i < ast_len(list); i++) {
        typecheck_stmt(nest, ast_at(list, i));
    }
}

static type_t *get_digit_type(node_t *expr) {
    (void)expr;
    return INT_TYPE;
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
    
        if (!convertible_to(to, from)) {
            reportf(LEVEL_ERROR, arg, "incompatible argument type");
        }
    }

    return body->result;
}

static type_t *typecheck_func(sema_t *sema, node_t *decl) {
    type_t *result = resolve_symbol(sema, decl->result);
    size_t len = ast_len(decl->params);
    types_t *args = new_typelist(len);

    for (size_t i = 0; i < len; i++) {
        node_t *param = ast_at(decl->params, i);
        type_t *arg = typecheck_decl(sema, param);
        typelist_put(args, i, arg);
    }

    return new_callable(decl, args, result);
}

static type_t *typecheck_binary(sema_t *sema, node_t *expr) {
    type_t *lhs = typecheck_expr(sema, expr->lhs);
    type_t *rhs = typecheck_expr(sema, expr->rhs);
    binary_t op = expr->binary;
    type_t *result;

    if (is_comparison_op(op)) {
        result = BOOL_TYPE;

        if (!is_integer(lhs) || !is_integer(rhs)) {
            reportf(LEVEL_ERROR, expr, "both sides of comparison operation must be integral");
        }
    } else if (is_math_op(op)) {
        result = lhs;

        if (!is_integer(lhs) || !is_integer(rhs)) {
            reportf(LEVEL_ERROR, expr, "both sides of math operation must be integral");
        }
    } else {
        result = new_poison(expr, "unknown operation");
    }

    return result;
}

static type_t *typecheck_unary(sema_t *sema, node_t *expr) {
    type_t *type = typecheck_expr(sema, expr->expr);
    unary_t op = expr->unary;

    switch (op) {
    case UNARY_ABS: case UNARY_NEG:
        if (!is_integer(type)) {
            reportf(LEVEL_ERROR, expr, "unary operation requires integral");
        }
        break;
    case UNARY_TRY:
        reportf(LEVEL_INTERNAL, expr, "unimplemented unary operation");
        break;
    }

    return type;
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

    default:
        reportf(LEVEL_INTERNAL, expr, "unimplemented expression typecheck");
        type = new_poison(expr, "unimplemented expression typecheck");
        break;
    }

    connect_type(expr, type);

    return type;
}

static type_t *typecheck_decl(sema_t *sema, node_t *decl) {
    type_t *type = raw_type(decl);

    if (type) {
        return type;
    }

    switch (decl->kind) {
    case AST_DECL_PARAM:
        type = resolve_symbol(sema, decl->type);
        break;

    case AST_DECL_FUNC:
        type = typecheck_func(sema, decl);
        break;

    default:
        reportf(LEVEL_INTERNAL, decl, "unimplement declaration typecheck");
        type = new_poison(decl, "unimplement declaration typecheck");
        break;
    }

    connect_type(decl, type);
    return type;
}

static void typecheck_stmt(sema_t *sema, node_t *stmt) {
    type_t *type = VOID_TYPE;

    switch (stmt->kind) {
    case AST_RETURN:
        type = typecheck_return(sema, stmt);
        break;

    case AST_STMTS:
        typecheck_stmts(sema, stmt);
        break;

    case AST_DIGIT: case AST_UNARY: 
    case AST_BINARY: case AST_CALL:
        type = typecheck_expr(sema, stmt);
        if (!is_void(type)) {
            reportf(LEVEL_WARNING, stmt, "discarding value of expression");
        }
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
        
        add_decl_unique(sema, param);

        if (is_void(type)) {
            reportf(LEVEL_ERROR, param, "parameter cannot have void type");
        }
    }
}

static void validate_function(sema_t *sema, node_t *func) {
    sema_t *nest = new_sema(sema);
    validate_params(nest, func->params);
    sema->result = resolve_symbol(sema, func->result);

    typecheck_stmts(nest, func->body);
}

static void add_all_decls(sema_t *sema, nodes_t *decls) {
    size_t len = ast_len(decls);

    for (size_t i = 0; i < len; i++) {
        node_t *decl = ast_kind_at(decls, i, AST_DECL_FUNC);
        typecheck_func(sema, decl);
        add_decl_unique(sema, decl);
    }
}

static void typecheck_all_decls(sema_t *sema, nodes_t *decls) {
    add_all_decls(sema, decls);

    size_t len = ast_len(decls);

    for (size_t i = 0; i < len; i++) {
        node_t *decl = ast_kind_at(decls, i, AST_DECL_FUNC);
        validate_function(sema, decl);
    }
}

static void add_builtin(type_t *type) {
    add_decl(ROOT_SEMA, type->node);
}

/**
 * external api
 */

void typecheck(nodes_t *nodes) {
    sema_t *sema = new_sema(ROOT_SEMA);
    typecheck_all_decls(sema, nodes);
}

void sema_init(void) {
    ROOT_SEMA = new_sema(NULL);

    INT_TYPE = new_integer(INTEGER_INT, true, "int");
    UINT_TYPE = new_integer(INTEGER_INT, false, "uint");
    BOOL_TYPE = new_builtin(TYPE_BOOLEAN, "bool");
    VOID_TYPE = new_builtin(TYPE_VOID, "void");

    add_builtin(INT_TYPE);
    add_builtin(UINT_TYPE);
    add_builtin(BOOL_TYPE);
    add_builtin(VOID_TYPE);
}
