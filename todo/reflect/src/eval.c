#include "ref/eval.h"
#include "cthulhu/events/events.h"
#include "notify/notify.h"
#include "ref/ast.h"

#include "base/panic.h"

static eval_result_t eval_inner(mpz_t result, logger_t *logs, ref_ast_t *expr);

static eval_result_t eval_binary_expr(mpz_t result, logger_t *logs, ref_ast_t *expr)
{
    mpz_t lhs;
    mpz_t rhs;

    eval_result_t lhs_result = eval_inner(lhs, logs, expr->lhs);
    if (lhs_result != eEvalOk) return lhs_result;

    eval_result_t rhs_result = eval_inner(rhs, logs, expr->rhs);
    if (rhs_result != eEvalOk) return rhs_result;

    mpz_init_set_ui(result, 0);

    switch (expr->binary)
    {
    case eBinaryAdd:
        mpz_add(result, lhs, rhs);
        break;

    case eBinarySub:
        mpz_sub(result, lhs, rhs);
        break;

    case eBinaryMul:
        mpz_mul(result, lhs, rhs);
        break;

    case eBinaryDiv:
        if (mpz_cmp_ui(rhs, 0) == 0)
        {
            msg_notify(logs, &kEvent_DivideByZero, expr->node, "division by zero");
            return eEvalInvalid;
        }

        if (!mpz_divisible_p(lhs, rhs))
        {
            msg_notify(logs, &kEvent_InexactIntegerDivision, expr->node, "division of %s and %s would not be exact", mpz_get_str(NULL, 10, lhs), mpz_get_str(NULL, 10, rhs));
        }

        mpz_divexact(result, lhs, rhs);
        break;

    case eBinaryRem:
        if (mpz_cmp_ui(rhs, 0) == 0)
        {
            msg_notify(logs, &kEvent_DivideByZero, expr->node, "modulo by zero");
            return eEvalInvalid;
        }

        mpz_mod(result, lhs, rhs);
        break;

    case eBinaryBitAnd:
        mpz_and(result, lhs, rhs);
        break;
    case eBinaryBitOr:
        mpz_ior(result, lhs, rhs);
        break;
    case eBinaryXor:
        mpz_xor(result, lhs, rhs);
        break;

    case eBinaryShl:
        if (mpz_cmp_si(rhs, 0) < 0)
        {
            msg_notify(logs, &kEvent_ShiftByNegative, expr->node, "shift by negative amount");
            return eEvalInvalid;
        }

        mpz_mul_2exp(result, lhs, mpz_get_si(rhs));
        break;
    case eBinaryShr:
        if (mpz_cmp_si(rhs, 0) < 0)
        {
            msg_notify(logs, &kEvent_ShiftByNegative, expr->node, "shift by negative amount");
            return eEvalInvalid;
        }

        mpz_fdiv_q_2exp(result, lhs, mpz_get_si(rhs));
        break;

    default:
        NEVER("unknown binary operator %d", expr->binary);
    }

    return eEvalOk;
}

static eval_result_t eval_unary(mpz_t result, logger_t *logs, ref_ast_t *expr)
{
    mpz_t inner;

    eval_result_t inner_result = eval_inner(inner, logs, expr->expr);
    if (inner_result != eEvalOk) return inner_result;

    mpz_init_set_ui(result, 0);

    switch (expr->unary)
    {
    case eUnaryAbs:
        mpz_abs(result, inner);
        break;

    case eUnaryNeg:
        mpz_neg(result, inner);
        break;

    case eUnaryFlip:
        mpz_com(result, inner);
        break;

    case eUnaryNot:
        mpz_set_si(result, mpz_cmp_si(inner, 0) == 0);
        break;

    default:
        NEVER("unknown unary operator %d", expr->unary);
    }

    return eEvalOk;
}

static eval_result_t eval_inner(mpz_t result, logger_t *logs, ref_ast_t *expr)
{
    switch (expr->kind)
    {
    case eAstInteger:
        mpz_init_set(result, expr->integer);
        return eEvalOk;

    case eAstBinary:
        return eval_binary_expr(result, logs, expr);

    case eAstUnary:
        return eval_unary(result, logs, expr);

    case eAstOpaque:
        return eEvalOpaque;

    case eAstIdent:
        msg_notify(logs, &kEvent_CannotEvalExpression, expr->node, "cannot evaluate identifiers");
        return eEvalInvalid;

    default:
        msg_notify(logs, &kEvent_CannotEvalExpression, expr->node, "unknown node kind %d", expr->kind);
        return eEvalInvalid;
    }
}

eval_result_t eval_expr(mpz_t result, logger_t *logs, ref_ast_t *expr)
{
    CTASSERT(logs != NULL);
    CTASSERT(expr != NULL);

    return eval_inner(result, logs, expr);
}
