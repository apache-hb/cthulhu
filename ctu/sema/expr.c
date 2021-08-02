static void mark_string(node_t *str) {
    str->local = strings++;
}

static type_t *query_func(sema_t *sema, node_t *func) {
    type_t *type = raw_type(func);
    if (type) {
        return type;
    }

    type_t *result = query_type(sema, func->result);

    size_t len = list_len(func->params);
    list_t *args = new_list(NULL);

    for (size_t i = 0; i < len; i++) {
        node_t *param = list_at(func->params, i);
        type_t *param_type = query_type(sema, param->type);
        list_push(args, param_type);
    }

    return new_callable(func, args, result);
}

static bool is_var_recursive(node_t *node) {
    size_t len = list_len(current);
    for (size_t i = 0; i < len; i++) {
        if (list_at(current, i) == node) {
            return true;
        }
    }
    return false;
}

static type_t *delay_build_var(node_t *node) {
    if (is_var_recursive(node)) {
        reportf(LEVEL_ERROR, node, "recursive initialization of variable `%s`", get_decl_name(node));
        return new_poison(node, "recursive resolution");
    }

    type_t *ty = raw_type(node);
    if (ty) {
        return ty;
    }

    build_var(node->ctx, node);
    return get_type(node);
}

static type_t *query_local_expr(sema_t *sema, node_t *node, const char *name) {
    /**
     * search variables first
     */
    node_t *var = map_get(sema->vars, name);
    if (var) {
        mark_used(var);
        node->local = var->local;
        return delay_build_var(var);
    }

    /**
     * then search functions
     */
    node_t *func = map_get(sema->funcs, name);
    if (func) {
        mark_used(func);
        return query_func(sema, func);
    }

    if (sema->parent) {
        return query_local_expr(sema->parent, node, name);
    } else {
        /**
         * error out if we found nothing
         */
        reportf(LEVEL_ERROR, node, "unknown symbol `%s`", name);
        return new_poison(node, "unresolved symbol");
    }
}

/**
 * query for a variable or a function recursivley
 */
static type_t *query_expr_symbol(sema_t *sema, node_t *expr, list_t *symbol) {
    const char *first = list_first(symbol);
    
    /**
     * if the symbol length is 1 then it must be in 
     * the current namespace
     */
    if (symbol->len == 1) {
        return query_local_expr(sema, expr, first);
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
    type_t *ty = get_int_type(expr->sign, expr->integer);
    return ty;
}

static type_t *query_bool(void) {
    return BOOL_TYPE;
}

static type_t *query_string(node_t *str) {
    mark_string(str);
    return STRING_TYPE;
}

static bool is_lvalue(type_t *it) {
    return it->lvalue;
}

#include "unary.c"

static type_t *query_unary(sema_t *sema, node_t *expr) {
    switch (expr->unary) {
    case UNARY_ABS: case UNARY_NEG:
        return unary_math(sema, expr);
    case UNARY_REF:
        return unary_ref(sema, expr);
    case UNARY_DEREF:
        return unary_deref(sema, expr);
    case UNARY_TRY:
        reportf(LEVEL_ERROR, expr, "unary try is currently unimplemented");
        return new_poison(expr, "unimplemented");
    }

    assert("invalid unary");
    return new_poison(expr, "invalid unary expr");
}

#include "binary.c"

static type_t *query_binary(sema_t *sema, node_t *expr) {
    switch (expr->binary) {
    case BINARY_ADD: case BINARY_SUB: case BINARY_DIV:
    case BINARY_MUL: case BINARY_REM:
        return binary_math(expr->binary, sema, expr);

    case BINARY_LT: case BINARY_LTE:
    case BINARY_GT: case BINARY_GTE:
        return binary_cmp(sema, expr);

    case BINARY_EQ: case BINARY_NEQ:
        return binary_eq(sema, expr);

    case BINARY_AND: case BINARY_OR:
        return binary_logic(sema, expr);

    case BINARY_BITAND: case BINARY_BITOR:
    case BINARY_XOR:
        return binary_bit(sema, expr);

    case BINARY_SHL: case BINARY_SHR:
        return binary_shift(sema, expr);

    default:
        assert("unknown binary operator %d", expr->binary);
        return new_poison(expr, "unknown binary operator");
    }
}

static type_t *query_call(sema_t *sema, node_t *expr) {
    type_t *type = query_expr(sema, expr->expr);

    if (!is_callable(type)) {
        reportf(LEVEL_ERROR, expr, "uncallable type");
        return new_poison(expr, "not callable");
    }

    size_t args = list_len(expr->args);
    size_t needs = list_len(type->args);

    if (args != needs) {
        reportf(LEVEL_ERROR, expr, "wrong number of arguments, expected %zu, got %zu", needs, args);
    } else {
        for (size_t i = 0; i < args; i++) {
            node_t *arg = list_at(expr->args, i);
            type_t *in = query_expr(sema, arg);
            type_t *out = list_at(type->args, i);
            if (!type_can_become_implicit(&arg, in, out)) {
                reportid_t id = reportf(LEVEL_ERROR, arg, "argument %zu expected type %s", i, typefmt(out));
                report_underline(id, typefmt(in));
            }
        }
    }

    return type->result;
}

static type_t *query_cast(sema_t *sema, node_t *expr) {
    type_t *src = query_expr(sema, expr->expr);
    type_t *dst = query_type(sema, expr->cast);

    if (!type_can_become_explicit(&expr, dst, src)) {
        return new_poison(expr, "invalid cast");
    }

    return dst;
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

    expr->local = expr->target->local;

    return get_field(body, expr);
}

static type_t *query_index(sema_t *sema, node_t *expr) {
    type_t *body = query_expr(sema, expr->expr);
    type_t *index = query_expr(sema, expr->index);

    if (!can_index(body)) {
        reportf(LEVEL_ERROR, expr, "cannot index into non indexable type `%s`", typefmt(body));
        return new_poison(expr, "bad index");
    }

    if (!type_can_become_explicit(&expr->index, get_int_type(false, INTEGER_SIZE), index)) {
        reportid_t id = reportf(LEVEL_ERROR, expr->index, "index expected type %s", typefmt(get_int_type(false, INTEGER_SIZE)));
        report_underline(id, format("found `%s` instead", typefmt(index)));
    }

    connect_type(expr->expr, array_decay(body));

    mark_local(expr);

    type_t *out = index_type(body);
    return set_lvalue(out, true);
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
        type = query_string(it);
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
    case AST_INDEX:
        type = query_index(sema, it);
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