static type_t *query_type_symbol(sema_t *sema, node_t *expr, list_t *symbol) {
    const char *first = list_first(symbol);
    if (symbol->len == 1) {
        node_t *type = map_get(sema->types, first);
        if (type) {
            return get_type(type);
        }

        reportf(LEVEL_ERROR, expr, "unknown type `%s`", first);
        return new_poison(expr, format("unresolved type `%s`", first));
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
    return query_type_symbol(sema, it, it->ident);
}