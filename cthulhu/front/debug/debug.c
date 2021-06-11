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

static void debug_digit(debug_t *debug, node_t *node) {
    fprintf(debug->out, "%lu", node->digit);
}

static void debug_unary(debug_t *debug, node_t *node) {
    fprintf(debug->out, "%s ", unary_op(node->unary.op));
    debug_ast(debug, node->unary.expr);
}

static void debug_binary(debug_t *debug, node_t *node) {
    fprintf(debug->out, "%s ", binary_op(node->binary.op));
    debug_ast(debug, node->binary.lhs);
    fprintf(debug->out, " ");
    debug_ast(debug, node->binary.rhs);
}

static void debug_ternary(debug_t *debug, node_t *node) {
    fprintf(debug->out, "if ");
    debug_ast(debug, node->ternary.cond);
    fprintf(debug->out, " then ");
    debug_ast(debug, node->ternary.lhs);
    fprintf(debug->out, " else ");
    debug_ast(debug, node->ternary.rhs);
}

static void debug_return(debug_t *debug, node_t *node) {
    fprintf(debug->out, "return ");
    debug_ast(debug, node->expr);
}

void debug_ast(debug_t *debug, node_t *node) {
    fprintf(debug->out, "(");

    switch (node->type) {
    case AST_DIGIT: debug_digit(debug, node); break;
    case AST_UNARY: debug_unary(debug, node); break;
    case AST_BINARY: debug_binary(debug, node); break;
    case AST_TERNARY: debug_ternary(debug, node); break;
    case AST_RETURN: debug_return(debug, node); break;

    default: fprintf(debug->out, "ERROR"); break;
    }

    fprintf(debug->out, ")");
}
