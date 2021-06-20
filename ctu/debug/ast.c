#include "ast.h"

#include <stdio.h>
#include <inttypes.h>

static void debug_digit(node_t *node) {
    printf("digit %" PRIu64, node->digit);
}

static void debug_ident(node_t *node) {
    printf("ident %s", node->ident);
}

static const char *unary_name(unary_t unary) {
    switch (unary) {
    case UNARY_ABS: return "abs";
    case UNARY_NEG: return "neg";
    case UNARY_TRY: return "try";
    default: return "error";
    }
}

static void debug_unary(node_t *node) {
    printf("unary %s ", unary_name(node->unary));
    debug_ast(node->expr);
}

static const char *binary_name(binary_t binary) {
    switch (binary) {
    case BINARY_ADD: return "add";
    case BINARY_SUB: return "sub";
    case BINARY_MUL: return "mul";
    case BINARY_DIV: return "div";
    case BINARY_REM: return "rem";
    default: return "error";
    }
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

void debug_ast(node_t *node) {
    printf("(");
    switch (node->kind) {
    case AST_DIGIT: debug_digit(node); break;
    case AST_IDENT: debug_ident(node); break;
    case AST_UNARY: debug_unary(node); break;
    case AST_BINARY: debug_binary(node); break;
    case AST_RETURN: debug_return(node); break;
    case AST_STMTS: debug_stmts(node); break;
    default: printf("error"); break;
    }
    printf(")");
}
