#include "expr.h"

static lir_t *compile_digit(ctu_t *digit) {
    return lir_digit(digit->node, 
        /* type = */ type_literal_integer(), 
        /* digit = */ digit->digit
    );
}

static lir_t *compile_unary(sema_t *sema, ctu_t *expr) {
    lir_t *operand = compile_expr(sema, expr->operand);
    unary_t unary = expr->unary;

    if (!is_integer(lir_type(operand))) {
        report(sema->reports, ERROR, expr->node, "unary operator requires an integer operand");
    }

    return lir_unary(expr->node, lir_type(operand), unary, operand);
}

static lir_t *compile_binary(sema_t *sema, ctu_t *expr) {
    lir_t *lhs = compile_expr(sema, expr->lhs);
    lir_t *rhs = compile_expr(sema, expr->rhs);
    binary_t binary = expr->binary;

    const type_t *common = types_common(lir_type(lhs), lir_type(rhs));
    if (common == NULL) {
        report(sema->reports, ERROR, expr->node, "binary operation requires compatible operands");
    }

    return lir_binary(expr->node, common, binary, lhs, rhs);
}

lir_t *compile_expr(sema_t *sema, ctu_t *expr) {
    switch (expr->type) {
    case CTU_DIGIT: return compile_digit(expr);
    case CTU_UNARY: return compile_unary(sema, expr);
    case CTU_BINARY: return compile_binary(sema, expr);

    default:
        ctu_assert(sema->reports, "compile-expr unimplemented");
        return lir_poison(expr->node, "unimplemented");
    }
}
