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

static void debug_ident(node_t *node) {
    debugf("%s", node->text);
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

static void debug_call(node_t *node) {
    debugf("call ");
    debug_ast(node->expr);
}

static void debug_return(node_t *node) {
    debugf("return ");
    debug_ast(node->expr);
}

static void debug_func(node_t *node) {
    debugf("define %s ", node->func.name);
    debug_ast(node->func.result);
    debugf(" ");
    debug_ast(node->func.body);
}

static void debug_stmts(node_t *node) {
    debugf("stmts ");
    for (size_t i = 0; i < node->stmts->len; i++) {
        if (i) {
            debugf(" ");
        }
        debug_ast(node->stmts->data + i);
    }
}

static void debug_bool(node_t *node) {
    debugf("%s", node->b ? "true" : "false");
}

static void debug_typename(node_t *node) {
    debugf("typename `%s`", node->text);
}

static void debug_var(node_t *node) {
    debugf("var %s ", node->var.name);
    debug_ast(node->var.init);
}

static void debug_cast(node_t *node) {
    debugf("cast ");
    debug_ast(node->cast.expr);
    debugf(" to ");
    debug_ast(node->cast.type);
}

void debug_ast(node_t *node) {
    debugf("(");

    switch (node->type) {
    case AST_DIGIT: debug_digit(node); break;
    case AST_IDENT: debug_ident(node); break;
    case AST_BOOL: debug_bool(node); break;
    case AST_UNARY: debug_unary(node); break;
    case AST_BINARY: debug_binary(node); break;
    case AST_TERNARY: debug_ternary(node); break;
    case AST_CALL: debug_call(node); break;
    case AST_RETURN: debug_return(node); break;
    case AST_FUNC: debug_func(node); break;
    case AST_STMTS: debug_stmts(node); break;
    case AST_TYPENAME: debug_typename(node); break;
    case AST_VAR: debug_var(node); break;
    case AST_CAST: debug_cast(node); break;
    }

    debugf(")");
}
