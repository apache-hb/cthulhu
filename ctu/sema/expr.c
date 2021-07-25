
/**
 * query for a variable or a function recursivley
 */
static type_t *query_expr_symbol(sema_t *sema, node_t *expr, list_t *symbol) {
    printf("len: %zu\n", symbol->len);
    const char *first = list_first(symbol);
    
    /**
     * if the symbol length is 1 then it must be in 
     * the current namespace
     */
    if (symbol->len == 1) {
        /**
         * search variables first
         */
        node_t *var = map_get(sema->vars, first);
        if (var) {
            return get_type(var);
        }

        /**
         * then search functions
         */
        node_t *func = map_get(sema->funcs, first);
        if (func) {
            return get_type(func);
        }

        /**
         * error out if we found nothing
         */
        reportf(LEVEL_ERROR, expr, "unknown symbol `%s`", first);
        return new_poison(expr, format("unresolved symbol `%s`", first));
    }

    /**
     * search the imports to try and find it
     */
    sema_t *other = map_get(sema->imports, first);
    if (other) {
        list_t slice = list_slice(symbol, 1);
        return query_expr_symbol(other, expr, &slice);
    }

    /**
     * if everything fails then tell the user
     */
    reportf(LEVEL_ERROR, expr, "unknown symbol `%s`", first);
    return new_poison(expr, format("unresolved symbol `%s`", first));
}

static type_t *query_expr(sema_t *sema, node_t *it);

static type_t *query_digit(node_t *expr) {
    return get_int_type(expr->sign, expr->integer);
}

static type_t *query_bool(void) {
    return BOOL_TYPE;
}

static type_t *query_string(void) {
    return STRING_TYPE;
}

static bool is_lvalue(type_t *it) {
    return it->lvalue;
}

static type_t *unary_math(sema_t *sema, node_t *expr) {
    type_t *type = query_expr(sema, expr->expr);

    if (!is_integer(type)) {
        reportf(LEVEL_ERROR, expr, "unary operator on non-integer");
    }

    if (expr->unary == UNARY_NEG && !is_signed(type)) {
        reportf(LEVEL_ERROR, expr, "cannot negate unsigned int");
    }

    return type;
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
        reportf(LEVEL_ERROR, expr, "dereferencing non-pointer");
    }

    return inner;
}

static type_t *query_unary(sema_t *sema, node_t *expr) {
    switch (expr->unary) {
    case UNARY_ABS: case UNARY_NEG:
        return unary_math(sema, expr);
    case UNARY_REF:
        return unary_ref(sema, expr->expr);
    case UNARY_DEREF:
        return unary_deref(sema, expr->expr);
    case UNARY_TRY:
        assert("unary try is currently unimplemented");
        return new_poison(expr, "unary try is currently unimplemented");
    }
}

static type_t *binary_math(sema_t *sema, node_t *expr) {

}

static type_t *binary_cmp(sema_t *sema, node_t *expr) {

}

static type_t *binary_eq(sema_t *sema, node_t *expr) {

}

static type_t *query_binary(sema_t *sema, node_t *expr) {
    switch (expr->binary) {
    case BINARY_ADD: case BINARY_SUB: case BINARY_DIV:
    case BINARY_MUL: case BINARY_REM:
        return binary_math(sema, expr);

    case BINARY_LT: case BINARY_LTE:
    case BINARY_GT: case BINARY_GTE:
        return binary_cmp(sema, expr);

    case BINARY_EQ: case BINARY_NEQ:
        return binary_eq(sema, expr);

    default:
        assert("unknown binary operator %d", expr->binary);
        return new_poison(expr, "unknown binary operator");
    }
}

static type_t *query_call(sema_t *sema, node_t *expr) {
    type_t *type = query_expr(sema, expr->expr);
}

static type_t *query_cast(sema_t *sema, node_t *expr) {
    type_t *src = query_expr(sema, expr->expr);
    type_t *dst = query_type(sema, expr->cast);
}

static type_t *get_field(type_t *record, node_t *get) {
    for (size_t i = 0; i < record->fields.size; i++) {
        field_t field = record->fields.fields[i];

        if (strcmp(field.name, get->field) == 0) {
            return field.type;
        }
    }

    reportf(LEVEL_ERROR, get, "unknown field `%s`", get->field);
    return new_poison(get, "unknown field");
}

static type_t *query_access(sema_t *sema, node_t *expr) {
    type_t *body = query_expr(sema, expr->target);

    if (is_pointer(body)) {
        body = body->ptr;
        if (!expr->indirect) {
            reportf(LEVEL_ERROR, expr, "cannot access non pointer");
        }
    }

    if (!is_struct(body)) {
        reportf(LEVEL_ERROR, expr, "cannot access non struct type");
        return new_poison(expr, "cannot access non struct type");
    }

    return get_field(body, expr);
}

/**
 * given an expression find its type
 */
static type_t *query_expr(sema_t *sema, node_t *it) {
    type_t *type = raw_type(it);
    if (type) {
        return type;
    }

    switch (it->kind) {
    case AST_DIGIT: 
        type = query_digit(it);
        break;
    case AST_BOOL:
        type = query_bool();
        break;
    case AST_STRING:
        type = query_string();
        break;
    case AST_UNARY:
        type = query_unary(sema, it);
        break;
    case AST_BINARY:
        type = query_binary(sema, it);
        break;
    case AST_CALL:
        type = query_call(sema, it);
        break;
    case AST_CAST:
        type = query_cast(sema, it);
        break;
    case AST_ACCESS:
        type = query_access(sema, it);
        break;

    case AST_SYMBOL:
        type = query_expr_symbol(sema, it, it->ident);
        break;

    default:
        assert("query_expr invalid node %d", it->kind);
        type = new_poison(it, "invalid node");
        break;
    }

    connect_type(it, type);
    return type;
}