#include "ast.h"
#include "common.h"

static void debug_digit(node_t *node) {
    printf("digit %" PRIu64, node->digit);
}

static void debug_unary(node_t *node) {
    printf("unary %s ", unary_name(node->unary));
    debug_ast(node->expr);
}

static void debug_binary(node_t *node) {
    printf("binary %s ", binary_name(node->binary));
    debug_ast(node->lhs);
    printf(" ");
    debug_ast(node->rhs);
}

static void debug_return(node_t *node) {
    printf("return ");
    if (node->expr) {
        debug_ast(node->expr);
    } else {
        printf("void");
    }
}

static void debug_stmts(node_t *node) {
    printf("stmts[%zu]", node->stmts->len);
    if (node->stmts->len) {
        printf(" ");
    }
    for (size_t i = 0; i < node->stmts->len; i++) {
        if (i != 0) {
            printf(" ");
        }
        debug_ast(node->stmts->data + i);
    }
}

static void debug_call(node_t *node) {
    printf("call[%zu] ", node->args->len);
    debug_ast(node->expr);
    
    if (node->args->len) {
        printf(" ");
    }

    for (size_t i = 0; i < node->args->len; i++) {
        if (i != 0) {
            printf(" ");
        }
        debug_ast(node->args->data + i);
    }
}

static void debug_symbol(node_t *node) {
    printf("symbol %s", node->ident);
}

static void debug_func(node_t *node) {
    printf("def %s ", node->name);
    debug_ast(node->body);
}

void debug_ast(node_t *node) {
    printf("(");
    switch (node->kind) {
    case AST_DIGIT: debug_digit(node); break;
    case AST_UNARY: debug_unary(node); break;
    case AST_BINARY: debug_binary(node); break;
    case AST_RETURN: debug_return(node); break;
    case AST_STMTS: debug_stmts(node); break;
    case AST_CALL: debug_call(node); break;
    case AST_SYMBOL: debug_symbol(node); break;
    case AST_DECL_FUNC: debug_func(node); break;
    default: printf("error");
    }
    printf(")");
}
