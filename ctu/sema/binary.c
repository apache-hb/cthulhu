static type_t *binary_math_result(type_t *lhs, type_t *rhs) {
    integer_t width = MAX(lhs->integer, rhs->integer);

    bool sign = lhs->sign || rhs->sign;

    return get_int_type(sign, width);
}

static type_t *binary_math(sema_t *sema, node_t *expr) {
    type_t *lhs = query_expr(sema, expr->lhs);
    type_t *rhs = query_expr(sema, expr->rhs);

    if (!is_integer(lhs) || !is_integer(rhs)) {
        reportf(LEVEL_ERROR, expr, "binary math requires both operands to be integral");
        return new_poison(expr, "invalid binary");
    }

    return binary_math_result(lhs, rhs);
}

static type_t *binary_cmp(sema_t *sema, node_t *expr) {
    type_t *lhs = query_expr(sema, expr->lhs);
    type_t *rhs = query_expr(sema, expr->rhs);

    if (is_integer(lhs) && is_integer(rhs)) {
        if (lhs->sign != rhs->sign) {
            reportf(LEVEL_WARNING, expr, "sign mismatch in comparison");
        }
    }

    if (is_void(lhs) || is_void(rhs)) {
        reportf(LEVEL_ERROR, expr, "cannot compare void");
    }

    if (is_callable(lhs) || is_callable(rhs)) {
        reportf(LEVEL_ERROR, expr, "cannot compare functions");
    }

    if (is_struct(lhs) || is_struct(rhs)) {
        reportf(LEVEL_ERROR, expr, "cannot compare user defined types");
    }

    if (lhs->kind != rhs->kind) {
        reportf(LEVEL_ERROR, expr, "cannot compare incompatible types");
    }

    return query_bool();
}

static type_t *binary_eq(sema_t *sema, node_t *expr) {
    type_t *lhs = query_expr(sema, expr->lhs);
    type_t *rhs = query_expr(sema, expr->rhs);

    if (is_void(lhs) || is_void(rhs)) {
        reportf(LEVEL_ERROR, expr, "cannot use void in equality");
    }

    if (is_callable(lhs) || is_callable(rhs)) {
        reportf(LEVEL_ERROR, expr, "cannot use functions in equality");
    }

    if (is_struct(lhs) || is_struct(rhs)) {
        reportf(LEVEL_ERROR, expr, "cannot use user defined types in equality");
    }

    if (lhs->kind != rhs->kind) {
        reportf(LEVEL_ERROR, expr, "cannot use incompatible type in equality comparison");
    }

    return query_bool();
}
