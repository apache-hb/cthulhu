static type_t *unary_math(sema_t *sema, node_t *expr) {
    type_t *type = query_expr(sema, expr->expr);

    if (!is_integer(type)) {
        reportf(LEVEL_ERROR, expr, "unary operator on non-integer type `%s`", typefmt(type));
    }

    if (expr->unary == UNARY_NEG && !is_signed(type)) {
        reportf(LEVEL_ERROR, expr, "cannot negate unsigned type `%s`", typefmt(type));
    }

    return set_lvalue(type, false);
}

static type_t *unary_ref(sema_t *sema, node_t *expr) {
    type_t *type = query_expr(sema, expr->expr);
    if (!is_lvalue(type)) {
        reportf(LEVEL_ERROR, expr, "cannot take the address of a non-lvalue");
    }

    return new_pointer(expr, type);
}

static type_t *unary_deref(sema_t *sema, node_t *expr) {
    type_t *inner = query_expr(sema, expr->expr);

    if (is_pointer(inner)) {
        inner = inner->ptr;
    } else {
        reportf(LEVEL_ERROR, expr, "dereferencing non-pointer `%s`", typefmt(inner));
    }

    return set_lvalue(inner, true);
}