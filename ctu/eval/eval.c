#include "eval.h"

#include "ctu/util/report.h"

static bool eval_unary(mpz_t result, unary_t unary, node_t *expr) {
    bool out = eval_ast(result, expr);

    switch (unary) {
    case UNARY_ABS:
        mpz_abs(result, result);
        break;

    case UNARY_NEG:
        mpz_neg(result, result);
        break;

    default:
        assert("cannot consteval unary op %d", unary);
        return false;
    }

    return out;
}

bool eval_ast(mpz_t result, node_t *ast) {
    switch (ast->kind) {
    case AST_UNARY:
        return eval_unary(result, ast->unary, ast->expr);
    case AST_BINARY:
        return eval_binary(result, ast->binary, ast->lhs, ast->rhs);
    case AST_DIGIT:
        mpz_init_set(result, ast->num);
        return true;

    default:
        assert("cannot consteval node %d", ast->kind);
        return false;
    }
}
