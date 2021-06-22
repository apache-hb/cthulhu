#include "sema.h"

#include "ctu/util/report.h"
#include "ctu/util/str.h"
#include "ctu/util/util.h"

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
    return sema;
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

static type_t *new_poison(node_t *node, char *msg) {
    type_t *type = new_type(TYPE_POISON, false, false);
    type->text = msg;

    connect_type(node, type);

    return type;
}

/**
 * typechecking utils
 */

static bool is_binary_math_op(binary_t binary) {
    switch (binary) {
    case BINARY_ADD: case BINARY_SUB: 
    case BINARY_DIV: case BINARY_MUL:
    case BINARY_REM:
        return true;
    }
    return false;
}

static bool is_integer(type_t *type) {
    return type->kind == TYPE_INTEGER;
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
    switch (type->kind) {
    case TYPE_INTEGER: return type->node->nameof;
    case TYPE_POISON: return type->node->name;
    default: return "error";
    }
}

static bool types_equal(type_t *lhs, type_t *rhs) {
    if (lhs->kind != rhs->kind) {
        return false;
    }

    if (lhs->sign != rhs->sign) {
        return false;
    }

    if (lhs->kind == TYPE_INTEGER && rhs->kind == TYPE_INTEGER) {
        return lhs->integer == rhs->integer;
    }

    return false;
}

/**
 * constants
 */

static type_t *INT_TYPE;
static type_t *UINT_TYPE;

static sema_t *ROOT;

static node_t *find_decl(sema_t *sema, const char *name) {
    for (size_t i = 0; i < sema->decls->len; i++) {
        node_t *decl = sema->decls->data + i;
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
    for (size_t i = 0; i < decls->len; i++) {
        add_decl(sema, decls->data + i);
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
                "possible narrowing conversion when negating unsigned value of type %s", 
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

    if (is_binary_math_op(node->binary)) {
        if (!is_integer(lhs) || !is_integer(rhs)) {
            char *msg = format(
                "binary math operands must both be integers. %s and %s are incompatible",
                get_typename(lhs), get_typename(rhs)
            );

            reportf(LEVEL_ERROR, node, msg);

            return new_poison(node, msg);
        }
    }

    if (!types_equal(lhs, rhs)) {
        char *msg = format(
            "%s and %s are incompatible binary operands",
            get_typename(lhs), get_typename(rhs)
        );
        reportf(LEVEL_ERROR, node, msg);
        return new_poison(node, msg);
    }

    return copyof(lhs, sizeof(type_t));
}

static void typecheck_stmts(sema_t *sema, node_t *nodes, size_t len) {
    sema_t *nest = new_sema(sema);
    for (size_t i = 0; i < len; i++) {
        typecheck_node(nest, nodes + i);
    }
}

static type_t *typecheck_node(sema_t *sema, node_t *node) {
    type_t *type;

    if (node->typeof) {
        return node->typeof;
    }

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
        typecheck_node(sema, node->expr);
        return NULL;
    case AST_STMTS:
        typecheck_stmts(sema, node->stmts->data, node->stmts->len);
        return NULL;
    default:
        reportf(LEVEL_INTERNAL, node, "failed to typecheck node %d", node->kind);
        type = new_poison(node, format("failed to typechech node %d", node->kind));
        break;
    }

    connect_type(node, type);

    return type;
}

static void typecheck_decl(sema_t *sema, node_t *node) {
    ASSERT(node->kind == AST_DECL_FUNC)("typecheck_decl(kind = %d)", node->kind);
    sema_t *nest = new_sema(sema);

    typecheck_node(nest, node->body);
}

void typecheck(nodes_t *nodes) {
    sema_t *sema = new_sema(ROOT);
    add_decls(sema, nodes);

    for (size_t i = 0; i < nodes->len; i++) {
        typecheck_decl(sema, nodes->data + i);
    }
}

void sema_init(void) {
    ROOT = new_sema(NULL);

    INT_TYPE = new_integer(INTEGER_INT, true);
    UINT_TYPE = new_integer(INTEGER_INT, false);

    /**
     * add builtin types
     */
    add_decl(ROOT, ast_type("uint", UINT_TYPE));
    add_decl(ROOT, ast_type("int", INT_TYPE));
}
