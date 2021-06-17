#include "sema.h"

#include "cthulhu/util/report.h"
#include "cthulhu/front/compile.h"
#include "bison.h"

#include <string.h>

static nodes_t *ctx = NULL;

typedef struct {
    enum { BUILTIN } kind;
    const char *name;

    union {
        enum { LONG, BOOL, VOID } builtin;
    };
} type_t;

static type_t *new_builtin(int it, const char *name) {
    type_t *type = malloc(sizeof(type_t));
    type->kind = BUILTIN;
    type->name = name;
    type->builtin = it;
    return type;
}

static type_t *LONG_TYPE = NULL;
static type_t *BOOL_TYPE = NULL;
static type_t *VOID_TYPE = NULL;

static type_t *current_return_type = NULL;

static bool types_equal(type_t *lhs, type_t *rhs) {
    if (lhs->kind == BUILTIN && rhs->kind == BUILTIN) {
        return lhs->builtin == rhs->builtin;
    }

    return false;
}

static type_t *get_typename(node_t *node) {
    const char *name = node->text;

    if (strcmp(name, "long") == 0)
        return LONG_TYPE;
    else if (strcmp(name, "bool") == 0)
        return BOOL_TYPE;
    else if (strcmp(name, "void") == 0)
        return VOID_TYPE;
    
    add_error(format("unkonwn type `%s`", name), node->source, node->loc);
    return VOID_TYPE;
}

static type_t *typeof_node(node_t *node);

static bool is_type_bool_convertible(type_t *type) {
    return types_equal(type, BOOL_TYPE) || types_equal(type, LONG_TYPE);
}

/*
static bool is_bool_convertible(node_t *node) {
    return is_type_bool_convertible(typeof_node(node));
}*/

static type_t *typeof_node(node_t *node) {
    type_t *lhs, *rhs, *cond;
    switch (node->type) {
    case AST_DIGIT: return LONG_TYPE;
    case AST_BOOL: return BOOL_TYPE;
    case AST_BINARY: 
        lhs = typeof_node(node->binary.lhs);
        rhs = typeof_node(node->binary.rhs);
        if (!types_equal(lhs, rhs))
            add_error("both sides of a binary expression must have the same type", node->source, node->loc);
        return lhs;
    case AST_TERNARY:
        cond = typeof_node(node->ternary.cond);
        lhs = typeof_node(node->ternary.lhs);
        rhs = typeof_node(node->ternary.rhs);

        if (!types_equal(lhs, rhs)) {
            add_error("both sides of ternary must be the same type", node->source, node->loc);
            return VOID_TYPE;
        }

        if (!is_type_bool_convertible(cond)) {
            add_error("ternary condition must be convertible to a boolean", node->ternary.cond->source, node->ternary.cond->loc);
        }

        return lhs;
    case AST_TYPENAME:
        return get_typename(node);
    default:
        reportf("typeof_node(node->type = %d)", node->type);
        return VOID_TYPE;
    }
}

static bool node_types_equal(node_t *l, node_t *r) {
    type_t *lhs = typeof_node(l),
           *rhs = typeof_node(r);

    return types_equal(lhs, rhs);
}

static bool is_callable(node_t *node) {
    return node->type == AST_IDENT;
}

static node_t *find_name(const char *name) {
    for (size_t i = 0; i < ctx->len; i++) {
        node_t *node = ctx->data + i;
        ASSERT(node->type == AST_FUNC);

        if (strcmp(node->func.name, name) == 0)
            return node;
    }

    return NULL;
}

static bool is_integer_type(type_t *type) {
    return types_equal(type, LONG_TYPE);
}

static void sema_node(node_t *node);

static void sema_binary(node_t *node) {
    type_t *lhs = typeof_node(node->binary.lhs),
           *rhs = typeof_node(node->binary.rhs);

    bool ok = true;

    switch (node->binary.op) {
    case ADD: case SUB: case DIV: 
    case MUL: case REM:
        if (!is_integer_type(lhs) || !is_integer_type(rhs)) {
            add_error("binary math must be performed on integer types", node->source, node->loc);
            ok = false;
        }
        break;
    
    default:
        ok = false;
        reportf("sema_binary(node->binary.op = %d)", node->binary.op);
        break;
    }

    if (ok) {
        sema_node(node->binary.lhs);
        sema_node(node->binary.rhs);
    }
}

static void sema_node(node_t *node) {
    size_t i = 0;

    switch (node->type) {
    case AST_DIGIT: 
    case AST_IDENT: 
    case AST_BOOL:
        break;

    case AST_CALL: 
        if (!is_callable(node->expr)) {
            add_error(format("type `%s` is not callable", typeof_node(node->expr)->name), node->source, node->loc);
            break;
        }

        if (!find_name(node->expr->text)) {
            add_error(format("`%s` not declared", node->expr->text), node->expr->source, node->expr->loc);
        }
        break;

    case AST_UNARY:
        sema_node(node->unary.expr);
        break;

    case AST_BINARY:
        sema_binary(node);
        if (!node_types_equal(node->binary.lhs, node->binary.rhs))
            add_error("binary expression must have the same type on both sides", node->source, node->loc);

        sema_node(node->binary.lhs);
        sema_node(node->binary.rhs);
        break;

    case AST_RETURN:
        if (!node->expr) {
            if (!types_equal(current_return_type, VOID_TYPE)) {
                add_error("non-void function cannot return void", node->source, node->loc);
            }
        } else {
            if (!types_equal(current_return_type, typeof_node(node->expr))) {
                add_error("type mismatch in return", node->expr->source, node->expr->loc);
            }
        }
        break;

    /*
    case AST_TERNARY:
        sema_node(node->ternary.cond);
        sema_node(node->ternary.lhs);
        sema_node(node->ternary.rhs);

        if (!is_bool_convertible(node->ternary.cond))
            add_error("ternary condition must be convertible to a boolean", node->ternary.cond->source, node->ternary.cond->loc);
        
        if (!node_types_equal(node->ternary.lhs, node->ternary.rhs))
            add_error("both sides of ternary must be the same type", node->source, node->loc);
        
        break;
        */

    case AST_STMTS:
        for (i = 0; i < node->stmts->len; i++)
            sema_node(node->stmts->data + i);
        break;

    case AST_TERNARY:
        reportf("sema_node(AST_TERNARY)");
        break;

    case AST_FUNC:
        reportf("sema_node(AST_FUNC)");
        break;

    case AST_TYPENAME:
        reportf("sema_node(AST_TYPENAME)");
        break;
    }
}

static void sema_func(node_t *node) {
    /* functions shouldnt be nested with this setup */
    ASSERT(current_return_type == NULL);
    current_return_type = typeof_node(node->func.result);

    sema_node(node->func.body);

    current_return_type = NULL;
}

void sema_mod(nodes_t *nodes) {
    ctx = nodes;

    LONG_TYPE = new_builtin(LONG, "long");
    BOOL_TYPE = new_builtin(BOOL, "bool");
    VOID_TYPE = new_builtin(VOID, "void");

    for (size_t i = 0; i < nodes->len; i++) {
        sema_func(nodes->data + i);
    }
}
