#include "eval.h"

#include "ctu/util/report.h"

static bool eval_inner(mpz_t result, node_t *ast);

static bool eval_unary(mpz_t result, unary_t unary, node_t *expr) {
    bool out = eval_inner(result, expr);

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

static bool eval_binary(mpz_t result, binary_t binary, node_t *lhs, node_t *rhs) {
    mpz_t left;
    mpz_t right;

    bool l = eval_inner(left, lhs);
    bool r = eval_inner(right, rhs);
    switch (binary) {
    case BINARY_ADD:
        mpz_add(result, left, right);
        break;

    case BINARY_SUB:
        mpz_sub(result, left, right);
        break;

    case BINARY_MUL:
        mpz_mul(result, left, right);
        break;

    case BINARY_DIV:
        mpz_div(result, left, right);
        break;

    case BINARY_REM:
        mpz_mod(result, left, right);
        break;

    case BINARY_SHL:
        mpz_mul_2exp(result, left, mpz_get_ui(right));
        break;

    case BINARY_SHR:
        mpz_div_2exp(result, left, mpz_get_ui(right));
        break;

    case BINARY_XOR:
        mpz_xor(result, left, right);
        break;

    case BINARY_BITAND:
        mpz_and(result, left, right);
        break;

    case BINARY_BITOR:
        mpz_ior(result, left, right);
        break;

    default:
        assert("cannot consteval binary op %d", binary);
        return false;
    }

    return l && r;
}

static bool eval_inner(mpz_t result, node_t *ast) {
    switch (ast->kind) {
    case AST_UNARY:
        return eval_unary(result, ast->unary, ast->expr);
    case AST_BINARY:
        return eval_binary(result, ast->binary, ast->lhs, ast->rhs);
    case AST_DIGIT:
        mpz_init_set(result, ast->num);
        return true;
    case AST_CAST:
        return true;

    default:
        assert("cannot consteval node %d", ast->kind);
        return false;
    }
}

bool eval_ast(mpz_t result, node_t *ast) {
    bool out = eval_inner(result, ast);
    
    if (out) {
        /* for now this function is only for array sizes */
        sanitize_range(size_int(), 
            result, ast->scanner, ast->where
        );
    }

    return out;
}

static bool is_const_unary(unary_t unary) {
    return unary == UNARY_ABS || unary == UNARY_NEG;
}

bool is_consteval(node_t *node) {
    switch (node->kind) {
    case AST_DIGIT: case AST_STRING: case AST_BOOL:
    case AST_CAST:
        return true;

    case AST_UNARY: 
        return is_const_unary(node->unary)
            && is_consteval(node->expr);

    case AST_BINARY:
        return is_consteval(node->lhs)
            && is_consteval(node->rhs);

    default:
        return false;
    }
}
