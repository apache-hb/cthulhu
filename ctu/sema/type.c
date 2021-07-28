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
    return new_poison(expr, "unresolved type");
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
        type = set_lvalue(type, true);
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

static void validate_struct(type_t *type, type_t *member) {
    fields_t fields = type->fields;
    for (size_t i = 0; i < fields.size; i++) {
        field_t field = fields.fields[i];
        type_t *it = field.type;
        if (types_equal(it, member)) {
            reportf(LEVEL_ERROR, type->node, "struct contains itself recursivley");
        }

        validate_struct(it, member);
    }
}

static void add_field(sema_t *sema, size_t at, type_t *record, node_t *field) {
    const char *name = get_field_name(field);

    if (!is_discard_name(name)) {
        for (size_t i = 0; i < at; i++) {
            field_t it = record->fields.fields[i];
            const char *other = it.name;

            if (strcmp(name, other) == 0) {
                reportf(LEVEL_ERROR, field, "duplicate field `%s`", name);
            }
        }
    }

    field_t it = { name, query_type(sema, field->type) };

    record->fields.fields[at] = it;

    validate_struct(it.type, record);
}

static void build_struct(sema_t *sema, node_t *node) {
    list_t *fields = node->fields;
    size_t len = list_len(fields);

    type_t *result = new_struct(node, get_decl_name(node));
    resize_struct(result, len);

    for (size_t i = 0; i < len; i++) {
        node_t *field = list_at(fields, i);
        add_field(sema, i, result, field);
    }

    connect_type(node, result);
}
