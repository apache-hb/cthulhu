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

    return NULL;
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

static void typecheck_return(sema_t *sema, node_t *stmt) {
    node_t *expr = stmt->expr;

    type_t *result = expr == NULL 
        ? VOID_TYPE
        : typecheck_expr(sema, expr);

    type_t *expect = return_type(sema);

    if (!convertible_to(expect, result)) {
        reportf(LEVEL_ERROR, stmt, "incorrect return type");
    }
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

static type_t *typecheck_symbol(sema_t *sema, node_t *symbol) {
    const char *name = get_symbol_name(symbol);
    node_t *decl = get_decl(sema, name);

    return typecheck_decl(sema, decl);
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
        type_t *arg = typecheck_decl(sema, ast_at(decl->params, i));
        typelist_put(args, i, arg);
    }

    return new_callable(decl, args, result);
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

    case AST_SYMBOL: 
        type = typecheck_symbol(sema, expr);
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
        type = typecheck_decl(sema, decl->type);
        break;

    case AST_DECL_FUNC:
        type = typecheck_func(sema, decl);
        break;

    case AST_SYMBOL:
        type = typecheck_symbol(sema, decl);
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
    switch (stmt->kind) {
    case AST_RETURN:
        typecheck_return(sema, stmt);
        break;

    case AST_STMTS:
        typecheck_stmts(sema, stmt);
        break;

    default:
        reportf(LEVEL_INTERNAL, stmt, "unimplement statement typecheck");
        break;
    }
}

static void typecheck_func_params(sema_t *sema, nodes_t *params) {
    size_t len = ast_len(params);

    for (size_t i = 0; i < len; i++) {
        node_t *param = ast_kind_at(params, i, AST_DECL_PARAM);

        add_decl_unique(sema, param);
    }

    for (size_t i = 0; i < len; i++) {
        node_t *param = ast_kind_at(params, i, AST_DECL_PARAM);

        type_t *type = resolve_symbol(sema, param->type);

        if (is_void(type)) {
            reportf(LEVEL_ERROR, param, "parameter cannot have void type");
        }

        connect_type(param, type);
        connect_type(param->type, type);
    }
}

static void typecheck_func_result(sema_t *sema, node_t *result) {
    sema->result = resolve_symbol(sema, result);
}

static void typecheck_function_impl(sema_t *sema, node_t *decl) {
    sema_t *nested = new_sema(sema);

    typecheck_func_params(nested, decl->params);
    typecheck_func_result(nested, decl->result);

    typecheck_stmts(nested, decl->body);
}

static void add_all_decls(sema_t *sema, nodes_t *decls) {
    size_t len = ast_len(decls);

    for (size_t i = 0; i < len; i++) {
        node_t *decl = ast_kind_at(decls, i, AST_DECL_FUNC);
        add_decl_unique(sema, decl);
    }
}

static void typecheck_all_decls(sema_t *sema, nodes_t *decls) {
    add_all_decls(sema, decls);

    size_t len = ast_len(decls);

    for (size_t i = 0; i < len; i++) {
        node_t *decl = ast_kind_at(decls, i, AST_DECL_FUNC);
        typecheck_function_impl(sema, decl);
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

#if 0

#include "ctu/util/report.h"
#include "ctu/util/str.h"
#include "ctu/util/util.h"

#include "ctu/debug/ast.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * builders
 */

static sema_t *new_sema(sema_t *parent) {
    sema_t *sema = malloc(sizeof(sema_t));
    sema->parent = parent;
    sema->decls = ast_list(NULL);
    sema->current_return_type = NULL;
    return sema;
}

static type_t *current_return_type(sema_t *sema) {
    if (sema->current_return_type) {
        return sema->current_return_type;
    }

    if (sema->parent) {
        return current_return_type(sema->parent);
    }

    return NULL;
}

static void set_current_return_type(sema_t *sema, type_t *type) {
    sema->current_return_type = type;
}

static type_t *new_type(typeof_t kind, bool mut, bool sign) {
    type_t *type = malloc(sizeof(type_t));
    type->kind = kind;
    type->mut = mut;
    type->sign = sign;
    return type;
}

static type_t *new_integer(integer_t real, bool sign) {
    type_t *type = new_type(TYPE_INTEGER, false, sign);
    type->integer = real;
    return type;
}

static type_t *new_boolean(void) {
    type_t *type = new_type(TYPE_BOOLEAN, false, false);
    return type;
}

static type_t *new_poison(node_t *node, char *msg) {
    type_t *type = new_type(TYPE_POISON, false, false);
    type->text = msg;

    connect_type(node, type);

    return type;
}

static type_t *new_callable(node_t *function, type_t *result, types_t params) {
    type_t *type = new_type(TYPE_CALLABLE, false, false);
    type->function = function;
    type->result = result;
    type->args = params;

    return type;
}

static types_t new_typelist(size_t size) {
    types_t types = { malloc(sizeof(type_t) * size), size };
    return types;
}

/**
 * typechecking utils
 */

static bool is_math_op(binary_t binary) {
    switch (binary) {
    case BINARY_ADD: case BINARY_SUB: 
    case BINARY_DIV: case BINARY_MUL:
    case BINARY_REM:
        return true;
    default:
        return false;
    }
}

static bool is_comparison_op(binary_t binary) {
    switch (binary) {
    case BINARY_GT: case BINARY_GTE:
    case BINARY_LT: case BINARY_LTE:
        return true;
    default:
        return false;
    }
}

static bool is_integer(type_t *type) {
    return type->kind == TYPE_INTEGER;
}

static bool is_boolean(type_t *type) {
    return type->kind == TYPE_BOOLEAN;
}

static bool is_callable(type_t *type) {
    return type->kind == TYPE_CALLABLE;
}

static bool is_builtin(type_t *type) {
    return type->kind == TYPE_INTEGER 
        || type->kind == TYPE_BOOLEAN;
}

static bool is_signed(type_t *type) {
    return type->sign;
}

static type_t *make_signed(type_t *type) {
    type_t *out = copyof(type, sizeof(type_t));
    out->sign = true;
    return out;
}

static const char *get_typename(type_t *type) {
    if (!type) {
        return "null";
    }

    switch (type->kind) {
    case TYPE_INTEGER: return type->node->nameof;
    case TYPE_BOOLEAN: return "bool";
    case TYPE_POISON: return type->text;
    case TYPE_CALLABLE: return format("() -> %s", get_typename(type->result));
    default: return format("error %d", type->kind);
    }
}

static bool comatible_binary_operands(type_t *lhs, type_t *rhs) {
    if (is_integer(lhs) && is_integer(rhs)) {
        return true;
    }

    return false;
}

static bool integer_conversion_would_truncate(type_t *to, type_t *from) {
    return to->integer < from->integer;
}

static bool convertible_to(node_t *node, type_t *to, type_t *from, bool implicit_cast) {
    if (is_integer(to) && is_integer(from)) {
        if (!is_signed(to) && is_signed(from) && implicit_cast) {
            reportf(LEVEL_WARNING, node, 
                format("implicit conversion to type `%s` from signed type `%s` may be lossy",
                get_typename(to), get_typename(from)
            ));
        }

        if (integer_conversion_would_truncate(to, from) && implicit_cast) {
            reportf(LEVEL_WARNING, node,
                format("implicit conversion to type `%s` to type `%s` may truncate",
                get_typename(to), get_typename(from)
            ));
        }

        return true;
    }

    return false;
}

static bool implicity_converitble_to(node_t *node, type_t *to, type_t *from) {
    return convertible_to(node, to, from, true);
}

static bool is_implicit_boolean(type_t *type) {
    if (is_integer(type) || is_boolean(type)) {
        return true;
    }

    return false;
}

/**
 * constants
 */

static type_t *INT_TYPE;
static type_t *UINT_TYPE;

static type_t *BOOL_TYPE;

static sema_t *ROOT;

static node_t *find_decl(sema_t *sema, const char *name) {
    for (size_t i = 0; i < ast_len(sema->decls); i++) {
        node_t *decl = ast_at(sema->decls, i);
        if (strcmp(node_name(decl), name) == 0) {
            return decl;
        }
    }

    return !!sema->parent
        ? find_decl(sema->parent, name)
        : NULL;
}

/**
 * add a decl, dont allow name shadowing
 */
static void add_decl(sema_t *sema, node_t *decl) {
    const char *name = node_name(decl);

    if (find_decl(sema, name)) {
        reportf(LEVEL_ERROR, decl, "`%s` redefined", name);
    }

    ast_append(sema->decls, decl);
}

static void add_decls(sema_t *sema, nodes_t *decls) {
    for (size_t i = 0; i < ast_len(decls); i++) {
        add_decl(sema, ast_at(decls, i));
    }
}

static type_t *typecheck_node(sema_t *sema, node_t *node);

static type_t *typecheck_unary(sema_t *sema, node_t *node) {
    type_t *expr = typecheck_node(sema, node->expr);
    type_t *type;

    if (node->unary == UNARY_NEG) {
        type = make_signed(expr);

        if (!is_signed(expr)) {
            reportf(LEVEL_WARNING, 
                node, 
                "possible narrowing conversion when negating unsigned value of type `%s`", 
                get_typename(expr)
            );
        }
    } else {
        type = copyof(expr, sizeof(type_t));
    }

    return type;
}

static type_t *typecheck_binary(sema_t *sema, node_t *node) {
    type_t *lhs = typecheck_node(sema, node->lhs),
           *rhs = typecheck_node(sema, node->rhs);

    if (is_math_op(node->binary)) {
        if (!is_integer(lhs) || !is_integer(rhs)) {
            char *msg = format(
                "binary math operands must both be integers. %s and %s are incompatible",
                get_typename(lhs), get_typename(rhs)
            );

            reportf(LEVEL_ERROR, node, msg);

            return new_poison(node, msg);
        }
    } else if (is_comparison_op(node->binary)) {
        if (!is_integer(lhs) || !is_integer(rhs)) {
            char *msg = format("binary comparison operands must both be integers. %s and %s are incompatible",
                get_typename(lhs), get_typename(rhs)
            );

            reportf(LEVEL_ERROR, node, msg);

            return new_poison(node, msg);
        }

        return BOOL_TYPE;
    }

    if (!comatible_binary_operands(lhs, rhs)) {
        char *msg = format(
            "`%s` and `%s` are incompatible binary operands",
            get_typename(lhs), get_typename(rhs)
        );
        reportf(LEVEL_ERROR, node, msg);
        return new_poison(node, msg);
    }

    return copyof(lhs, sizeof(type_t));
}

static void typecheck_stmts(sema_t *sema, nodes_t *nodes) {
    sema_t *nest = new_sema(sema);
    for (size_t i = 0; i < ast_len(nodes); i++) {
        typecheck_node(nest, ast_at(nodes, i));
    }
}

static type_t *typecheck_call(sema_t *sema, node_t *node) {
    type_t *func = typecheck_node(sema, node->expr);
    if (!is_callable(func)) {
        reportf(LEVEL_ERROR, node, format("cannot call type %s", get_typename(func)));
        return new_poison(node->expr, "malformed call expression");
    }

    size_t len = ast_len(node->args);

    if (len != func->args.size) {
        reportf(LEVEL_ERROR, node, format("expected %zu arguments but %zu were provided",
            func->args.size, len
        ));
    }

    type_t *other = typecheck_node(sema, func->function);

    types_t args = new_typelist(len);

    for (size_t i = 0; i < len; i++) {
        node_t *arg = ast_at(node->args, i);
        type_t *input = typecheck_node(sema, arg);
        type_t *expected = other->args.data + i;

        if (!implicity_converitble_to(arg, expected, input)) {
            reportf(LEVEL_ERROR, arg, format("cannot implicity convert argument %zu from `%s` to `%s`",
                i, get_typename(input), get_typename(expected)
            ));
            break;
        }
    }

    func->args = args;

    return other->result;
}

static type_t *typecheck_func(node_t *node) {
    ASSERT(node->typeof != NULL)("malformed function in typecheck");
    printf("typecheck %p %p\n", node, node->typeof);
    return node->typeof;
}

static void typecheck_branch(sema_t *sema, node_t *node) {
    type_t *cond = typecheck_node(sema, node->cond);
    typecheck_node(sema, node->branch);

    if (!is_implicit_boolean(cond)) {
        reportf(LEVEL_ERROR, node->cond, 
            format("cannot implicity convert type `%s` to a boolean", get_typename(cond))
        );
    }
}

static void typecheck_return(sema_t *sema, node_t *node) {
    type_t *type = typecheck_node(sema, node->expr);
    type_t *result = current_return_type(sema);

    if (!implicity_converitble_to(node, result, type)) {
        reportf(LEVEL_ERROR, node, 
            format("cannot return %s from a function that returns %s",
            get_typename(type), get_typename(result)   
        ));
    }
}

static type_t *typecheck_node(sema_t *sema, node_t *node) {
    ASSERT(node != NULL)("cannot typecheck NULL");
    
    type_t *type;
    node_t *decl;

    if (node->typeof) {
        return node->typeof;
    }
    
    printf("type %d\n", node->kind);

    switch (node->kind) {
    case AST_DIGIT: 
        type = INT_TYPE; 
        break;
    case AST_UNARY: 
        type = typecheck_unary(sema, node);
        break;
    case AST_BINARY:
        type = typecheck_binary(sema, node);
        break;
    case AST_RETURN:
        typecheck_return(sema, node);
        return NULL;
    case AST_STMTS:
        typecheck_stmts(sema, node->stmts);
        return NULL;
    case AST_SYMBOL:
        decl = find_decl(sema, node->ident);
        if (!decl) {
            reportf(LEVEL_ERROR, node, format("cannot resolve symbol `%s`", node->ident));
            type = new_poison(node, format("unresolved symbol `%s`", node->ident));
            break;
        }
        type = typecheck_node(sema, decl);
        break;
    case AST_CALL:
        type = typecheck_call(sema, node);
        break;
    case AST_DECL_FUNC:
        type = typecheck_func(node);
        printf("decl %s\n", get_typename(type));
        break;
    case AST_DECL_PARAM:
        type = typecheck_node(sema, node->type);
        break;
    case AST_BRANCH:
        typecheck_branch(sema, node);
        return NULL;
    default:
        reportf(LEVEL_INTERNAL, node, "failed to typecheck node %d", node->kind);
        type = new_poison(node, format("failed to typechech node %d", node->kind));
        break;
    }

    if (is_builtin(type)) {
        node->typeof = type;
    } else {
        connect_type(node, type);
    }

    return type;
}

/**
 * create a function
 * typecheck its parameters and return type 
 * then attach type info to node
 * 
 * must be called *after* all decls have been added
 * to make sure order independant declaration works
 */
/*
static void build_function(sema_t *sema, node_t *func) {

}*/

static types_t add_params(sema_t *sema, nodes_t *params) {
    size_t len = ast_len(params);
    types_t out = new_typelist(len);

    for (size_t i = 0; i < len; i++) {
        node_t *param = ast_at(params, i);

        type_t *type = typecheck_node(sema, param->type);
        memcpy(out.data + i, type, sizeof(type_t));

        add_decl(sema, param);
    }

    return out;
}

static void typecheck_decl(sema_t *sema, node_t *node) {
    ASSERT(node->kind == AST_DECL_FUNC)("typecheck_decl(kind = %d)", node->kind);
    sema_t *nest = new_sema(sema);

    set_current_return_type(sema, node->typeof->result);

    typecheck_node(nest, node->body);
}

static void typecheck_params(sema_t *sema, node_t *node) {
    type_t *result = typecheck_node(sema, node->result);
    types_t params = add_params(sema, node->params);
    node->typeof = new_callable(node, result, params);
}

void typecheck(nodes_t *nodes) {
    sema_t *sema = new_sema(ROOT);
    add_decls(sema, nodes);

    size_t len = ast_len(nodes);

    for (size_t i = 0; i < len; i++) {
        typecheck_params(sema, ast_at(nodes, i));
    }

    for (size_t i = 0; i < len; i++) {
        typecheck_decl(sema, ast_at(nodes, i));
    }
}

void sema_init(void) {
    ROOT = new_sema(NULL);

    INT_TYPE = new_integer(INTEGER_INT, true);
    UINT_TYPE = new_integer(INTEGER_INT, false);

    BOOL_TYPE = new_boolean();

    /**
     * add builtin types
     */
    add_decl(ROOT, ast_type("uint", UINT_TYPE));
    add_decl(ROOT, ast_type("int", INT_TYPE));
    add_decl(ROOT, ast_type("bool", BOOL_TYPE));
}
#endif