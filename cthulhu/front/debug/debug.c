#include "debug.h"
#include "bison.h"

static const char *unary_op(int op) {
    switch (op) {
    case ADD: return "abs";
    case SUB: return "neg";
    default: return "ERROR";
    }
}

static const char *binary_op(int op) {
    switch (op) {
    case ADD: return "add";
    case SUB: return "sub";
    case DIV: return "div";
    case MUL: return "mul";
    case REM: return "rem";
    default: return "ERROR";
    }
}

static void debug_digit(node_t *node) {
    debugf("%lu", node->digit);
}

static void debug_unary(node_t *node) {
    debugf("%s ", unary_op(node->unary.op));
    debug_ast(node->unary.expr);
}

static void debug_binary(node_t *node) {
    debugf("%s ", binary_op(node->binary.op));
    debug_ast(node->binary.lhs);
    debugf(" ");
    debug_ast(node->binary.rhs);
}

static void debug_ternary(node_t *node) {
    debugf("if ");
    debug_ast(node->ternary.cond);
    debugf(" then ");
    debug_ast(node->ternary.lhs);
    debugf(" else ");
    debug_ast(node->ternary.rhs);
}

static void debug_return(node_t *node) {
    debugf("return ");
    debug_ast(node->expr);
}

void debug_ast(node_t *node) {
    debugf("(");

    switch (node->type) {
    case AST_DIGIT: debug_digit(node); break;
    case AST_UNARY: debug_unary(node); break;
    case AST_BINARY: debug_binary(node); break;
    case AST_TERNARY: debug_ternary(node); break;
    case AST_RETURN: debug_return(node); break;

    default: debugf("ERROR"); break;
    }

    debugf(")");
}
