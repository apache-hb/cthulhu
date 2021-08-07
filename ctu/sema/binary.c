static type_t *binary_math_result(type_t *lhs, type_t *rhs) {
    integer_t width = MAX(lhs->integer, rhs->integer);

    bool sign = lhs->sign || rhs->sign;

    return get_int_type(sign, width);
}

static type_t *binary_math(binary_t binary, sema_t *sema, node_t *expr) {
    type_t *lhs = query_expr(sema, expr->lhs);
    type_t *rhs = query_expr(sema, expr->rhs);

    if (!is_integer(rhs)) {
        reportf(LEVEL_ERROR, expr, "binary math requires right side operand to be integral. `%s` is not integral", typefmt(rhs));
        return new_poison(expr, "invalid binary");
    }

    if (binary == BINARY_ADD || binary == BINARY_SUB) {
        if (!is_integer(lhs) && !is_pointer(lhs)) {
            reportf(LEVEL_ERROR, expr, "binary addition is only valid on integral and pointer types. `%s` is neither", typefmt(lhs));
            return new_poison(expr, "invalid binary");
        }
    }

    if (!is_integer(lhs)) {
        reportf(LEVEL_ERROR, expr, "binary math requires left side operand to be integral. `%s` is not integral", typefmt(lhs));
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

    if (is_record(lhs) || is_record(rhs)) {
        reportf(LEVEL_ERROR, expr, "cannot compare user defined types");
    }

    if (lhs->kind != rhs->kind) {
        reportf(LEVEL_ERROR, expr, "cannot compare incompatible types");
    }

    return BOOL_TYPE;
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

    if (is_record(lhs) || is_record(rhs)) {
        reportf(LEVEL_ERROR, expr, "cannot use user defined types in equality");
    }

    if (lhs->kind != rhs->kind) {
        reportid_t id = reportf(LEVEL_ERROR, expr, "cannot use incompatible type in equality comparison");
        report_underline(id, format("comparing %s to %s", typefmt(lhs), typefmt(rhs)));
    }

    return BOOL_TYPE;
}

static bool is_bool_convertible(node_t *node, type_t *type) {
    return type_can_become_implicit(&node, type, BOOL_TYPE);
}

static type_t *binary_logic(sema_t *sema, node_t *expr) {
    type_t *lhs = query_expr(sema, expr->lhs);
    type_t *rhs = query_expr(sema, expr->rhs);

    if (!is_bool_convertible(expr->lhs, lhs) || !is_bool_convertible(expr->rhs, rhs)) {
        reportf(LEVEL_ERROR, expr, "both sides of logical operation must be nool convertible");
    }

    return BOOL_TYPE;
}

static type_t *binary_bit(sema_t *sema, node_t *expr) {
    type_t *lhs = query_expr(sema, expr->lhs);
    type_t *rhs = query_expr(sema, expr->rhs);

    if (!is_integer(lhs) || !is_integer(rhs)) {
        reportf(LEVEL_ERROR, expr, 
            "both sides of bitwise operation must be integral, got `%s` and `%s`", 
            typefmt(lhs), typefmt(rhs)
        );
        return new_poison(expr, "invalid bitwise op");
    }

    return binary_math_result(lhs, rhs);
}

static type_t *binary_shift(sema_t *sema, node_t *expr) {
    type_t *lhs = query_expr(sema, expr->lhs);
    type_t *rhs = query_expr(sema, expr->rhs);

    if (!is_integer(lhs) || !is_integer(rhs)) {
        reportf(LEVEL_ERROR, expr, "both sides of shift operation must be integral, got `%s` and `%s`",
            typefmt(lhs), typefmt(rhs)   
        );
        return new_poison(expr, "invalid bitshift");
    }

    return binary_math_result(lhs, rhs);
}
