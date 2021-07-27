static type_t *query_type_local(sema_t *sema, node_t *node, const char *name) {
    node_t *type = map_get(sema->types, name);
    if (type) {
        return get_type(type);
    }

    type_t *builtin = map_get(builtins, name);
    if (builtin) {
        return builtin;
    }

    reportf(LEVEL_ERROR, node, "unknown type `%s`", name);
    return new_poison(node, format("unresolved type `%s`", name));
}

static type_t *query_type_symbol(sema_t *sema, node_t *expr, list_t *symbol) {
    const char *first = list_first(symbol);
    if (symbol->len == 1) {
        return query_type_local(sema, expr, first);
    }

    sema_t *other = map_get(sema->imports, first);
    if (other) {
        list_t slice = list_slice(symbol, 1);
        return query_type_symbol(other, expr, &slice);
    }

    reportf(LEVEL_ERROR, expr, "unknown type `%s`", first);
    return new_poison(expr, format("unresolved type `%s`", first));
}

/**
 * find a type given a typename
 */
static type_t *query_type(sema_t *sema, node_t *it) {
    type_t *type = raw_type(it); 
    if (type) {
        return type;
    }

    switch (it->kind) {
    case AST_SYMBOL:
        type = query_type_symbol(sema, it, it->ident);
        break;

    case AST_DECL_PARAM:
        type = query_type(sema, it->type);
        break;

    case AST_PTR:
        type = query_type(sema, it->ptr);
        type = new_pointer(it, type);
        break;

    case AST_MUT:
        type = query_type(sema, it->next);
        type = set_mut(type, true);
        break;

    default:
        assert("query-type invalid %d", it->kind);
        break;
    }

    connect_type(it, type);

    return type;
}
