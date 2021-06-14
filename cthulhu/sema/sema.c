#include "sema.h"

#include "cthulhu/util/report.h"

#include <string.h>

static nodes_t *ctx = NULL;

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

static char *pretty_print(node_t *node) {
    switch (node->type) {
    case AST_DIGIT: 
        return format("digit `%d`", node->digit);

    default:
        reportf("pretty_print(node->type = %d)", node->type);
        return NULL;
    }
}

static void sema_node(node_t *node) {
    switch (node->type) {
    case AST_DIGIT: break;
    case AST_IDENT: break;

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
        break;

    default:
        reportf("sema_node(node->type = %d)", node->type);
        break;
    }
}

static void sema_func(node_t *node) {
    sema_node(node->func.body);
}

void sema_mod(nodes_t *nodes) {
    ctx = nodes;
    for (size_t i = 0; i < nodes->len; i++) {
        sema_func(nodes->data + i);
    }
}
