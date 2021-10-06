#include "expr.h"
#include "value.h"
#include "define.h"

#include "ctu/type/retype.h"

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

static lir_t *compile_call(sema_t *sema, ctu_t *expr) {
    size_t len = vector_len(expr->args);
    lir_t *func = compile_expr(sema, expr->func);
    vector_t *args = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        ctu_t *it = vector_get(expr->args, i);
        lir_t *arg = compile_expr(sema, it);
        vector_set(args, i, arg);
    }

    const type_t *fty = lir_type(func);
    if (!is_closure(fty)) {
        report(sema->reports, ERROR, expr->node, "cannot apply call to type `%s`", type_format(fty));
        return lir_poison(expr->node, "invalid call");
    }

    size_t total = maximum_params(fty);
    if (total < len) {
        if (is_variadic(fty)) {
            size_t least = minimum_params(fty);
            report(sema->reports, ERROR, expr->node, "incorrect number of arguments, expected at least %zu have %zu", least, len);
        } else {
            report(sema->reports, ERROR, expr->node, "incorrect number of arguments, expected %zu have %zu", total, len);
        }

        return lir_poison(expr->node, "invalid call");
    }

    for (size_t i = 0; i < len; i++) {
        const type_t *want = param_at(fty, i);
        lir_t *arg = vector_get(args, i);
        lir_t *res = retype_expr(sema->reports, want, arg);

        if (is_poison(lir_type(res))) {
            report(sema->reports, ERROR, res->node, "cannot convert cast argument %zu from `%s` to `%s`", i, type_format(lir_type(arg)), type_format(want));
        }

        vector_set(args, i, res);
    }

    return lir_call(expr->node, closure_result(fty), func, args);
}

static lir_t *compile_name(sema_t *sema, ctu_t *expr) {
    const char *name = expr->ident;
    if (is_discard(name)) {
        report(sema->reports, WARNING, expr->node, "reading from a discarded identifier `%s`", name);
    }
    
    lir_t *var = get_var(sema, name);
    if (var != NULL) {
        lir_t *it = compile_value(var);
        return lir_name(expr->node, lir_type(it), it);
    }

    lir_t *func = get_func(sema, name);
    if (func != NULL) {
        lir_t *it = compile_define(func);
        return it;
    }

    report(sema->reports, ERROR, expr->node, "failed to resolve `%s`", name);
    return lir_poison(expr->node, "unresolved name");
}

lir_t *compile_expr(sema_t *sema, ctu_t *expr) {
    switch (expr->type) {
    case CTU_DIGIT: return compile_digit(expr);
    case CTU_UNARY: return compile_unary(sema, expr);
    case CTU_BINARY: return compile_binary(sema, expr);
    case CTU_CALL: return compile_call(sema, expr);
    case CTU_IDENT: return compile_name(sema, expr);

    default:
        ctu_assert(sema->reports, "(ctu) compile-expr unimplemented expr type %d", expr->type);
        return lir_poison(expr->node, "unimplemented");
    }
}

lir_t *compile_stmts(sema_t *sema, ctu_t *stmts) {
    vector_t *all = stmts->stmts;
    size_t len = vector_len(all);

    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        ctu_t *it = vector_get(stmts->stmts, i);
        lir_t *stmt = compile_stmt(sema, it);
        vector_set(result, i, stmt);
    }

    return lir_stmts(stmts->node, result);
}

static lir_t *compile_local(sema_t *sema, ctu_t *stmt) {
    lir_t *value = local_value(sema, stmt);
    add_local(sema, value);
    add_var(sema, stmt->name, value);
    return lir_assign(stmt->node, value, value->init);
}

static lir_t *compile_return(sema_t *sema, ctu_t *stmt) {
    lir_t *operand = stmt->operand == NULL ? NULL : compile_expr(sema, stmt->operand);

    return lir_return(stmt->node, operand);
}

static size_t SMALL_SIZES[TAG_MAX] = { MAP_SMALL, MAP_SMALL, MAP_SMALL };

lir_t *compile_stmt(sema_t *sema, ctu_t *stmt) {
    switch (stmt->type) {
    case CTU_STMTS: return compile_stmts(new_sema(sema->reports, sema, SMALL_SIZES), stmt);
    case CTU_RETURN: return compile_return(sema, stmt);

    case CTU_CALL: return compile_call(sema, stmt);
    case CTU_VALUE: return compile_local(sema, stmt);

    default: 
        ctu_assert(sema->reports, "compile-stmt unknown type %d", stmt->type);
        return lir_poison(stmt->node, "compile-stmt unknown type");
    }
}
