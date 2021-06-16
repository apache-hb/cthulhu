#include "sema.h"

#include "cthulhu/util/report.h"
#include "bison.h"

#include <string.h>

static nodes_t *ctx = NULL;

typedef struct {
    enum { BUILTIN } kind;

    union {
        enum { LONG, BOOL, VOID } builtin;
    };
} type_t;

static type_t *new_builtin(int it) {
    type_t *type = malloc(sizeof(type_t));
    type->kind = BUILTIN;
    type->builtin = it;
    return type;
}

static type_t *LONG_TYPE = NULL;
static type_t *BOOL_TYPE = NULL;
static type_t *VOID_TYPE = NULL;

static bool types_equal(type_t *lhs, type_t *rhs) {
    if (lhs->kind == BUILTIN && rhs->kind == BUILTIN) {
        return lhs->builtin == rhs->builtin;
    }

    return false;
}

static type_t *typeof_node(node_t *node) {
    type_t *lhs, *rhs;
    switch (node->type) {
    case AST_DIGIT: return LONG_TYPE;
    case AST_BOOL: return BOOL_TYPE;
    case AST_BINARY: 
        lhs = typeof_node(node->binary.lhs);
        rhs = typeof_node(node->binary.rhs);
        if (!types_equal(lhs, rhs))
            reportf("both sides of a binary expression must have the same type");
        return lhs;
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

static bool is_bool_convertible(node_t *node) {
    switch (node->type) {
    case AST_DIGIT: case AST_BOOL:
        return true;
    case AST_BINARY: 
        return is_bool_convertible(node->binary.lhs)
            && is_bool_convertible(node->binary.rhs);
    default:
        reportf("is_bool_convertible(node->type = %d)", node->type);
        return false;
    }
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

static char *pretty_print(node_t *node) {
    switch (node->type) {
    case AST_DIGIT: 
        return format("digit `%d`", node->digit);

    default:
        reportf("pretty_print(node->type = %d)", node->type);
        return NULL;
    }
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
            reportf("binary math must be performed on integer types");
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
        if (!is_callable(node->expr))
            reportf("%s is not callable", pretty_print(node->expr));

        if (!find_name(node->expr->text))
            reportf("`%s` not declared", node->expr->text);
        break;

    case AST_UNARY:
        sema_node(node->unary.expr);
        break;

    case AST_BINARY:
        sema_binary(node);
        if (!node_types_equal(node->binary.lhs, node->binary.rhs))
            reportf("binary expression must have the same type on both sides");

        sema_node(node->binary.lhs);
        sema_node(node->binary.rhs);
        break;

    case AST_RETURN:
        sema_node(node->expr);
        break;

    case AST_TERNARY:
        sema_node(node->ternary.cond);
        sema_node(node->ternary.lhs);
        sema_node(node->ternary.rhs);

        if (!is_bool_convertible(node->ternary.cond))
            reportf("ternary condition must be convertible to a boolean");
        
        if (!node_types_equal(node->ternary.lhs, node->ternary.rhs))
            reportf("both sides of ternary must be the same type");
        break;

    case AST_STMTS:
        for (i = 0; i < node->stmts->len; i++)
            sema_node(node->stmts->data + i);
        break;

    case AST_FUNC:
        reportf("sema_node(AST_FUNC)");
        break;
    }
}

static void sema_func(node_t *node) {
    sema_node(node->func.body);
}

void sema_mod(nodes_t *nodes) {
    ctx = nodes;

    LONG_TYPE = new_builtin(LONG);
    BOOL_TYPE = new_builtin(BOOL);
    VOID_TYPE = new_builtin(VOID);

    for (size_t i = 0; i < nodes->len; i++) {
        sema_func(nodes->data + i);
    }
}
