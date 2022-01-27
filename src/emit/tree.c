#include "cthulhu/emit/emit.h"

#include "cJSON.h"

typedef struct {
    map_t *types; // map of type to its index
    map_t *locals; // map of local to its index
    map_t *globals; // map of global to its index
} emit_t;

static const char *metatype_to_string(const type_t *type) {
    switch (type->type) {
    case TYPE_BOOLEAN: return "boolean";
    case TYPE_INTEGER: return "integer";
    case TYPE_STRING: return "string";
    case TYPE_VOID: return "void";
    case TYPE_SIGNATURE: return "signature";
    case TYPE_ERROR: return "error";
    default: return "unknown";
    }
}

static void add_location(cJSON *json, const node_t *node) {
    if (node == NULL) { return; }

    where_t where = node->where;

    cJSON *span = cJSON_CreateObject();
    cJSON_AddNumberToObject(span, "first-line", where.first_line);
    cJSON_AddNumberToObject(span, "first-column", where.first_column);
    cJSON_AddNumberToObject(span, "last-line", where.last_line);
    cJSON_AddNumberToObject(span, "last-column", where.last_column);

    cJSON_AddItemToObject(json, "span", span);
}

static size_t get_type(emit_t *emit, const type_t *type) {
    return (size_t)map_get_ptr(emit->types, type);
}

static cJSON *emit_type(emit_t *emit, size_t idx, const type_t *hlir) {
    map_set_ptr(emit->types, hlir, (void*)(uintptr_t)idx);

    cJSON *type = cJSON_CreateObject();
    cJSON_AddStringToObject(type, "type", metatype_to_string(hlir));
    cJSON_AddStringToObject(type, "name", hlir->name);

    if (type_is_signature(hlir)) {
        cJSON_AddNumberToObject(type, "result", get_type(emit, hlir->result));
        cJSON *params = cJSON_CreateArray();
        for (size_t i = 0; i < vector_len(hlir->params); i++) {
            const type_t *param = vector_get(hlir->params, i);
            cJSON_AddItemToArray(params, cJSON_CreateNumber(get_type(emit, param)));
        }
        cJSON_AddItemToObject(type, "params", params);
    }

    add_location(type, hlir->node);

    return type;
}

static cJSON *emit_value(emit_t *emit, const value_t *value) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "type", get_type(emit, value->type));

    if (type_is_boolean(value->type)) {
        cJSON_AddBoolToObject(json, "value", value->boolean);
    } else if (type_is_integer(value->type)) {
        cJSON_AddNumberToObject(json, "value", mpz_get_si(value->integer));
    } else if (type_is_string(value->type)) {
        cJSON_AddStringToObject(json, "value", value->string);
    } else {
        cJSON_AddStringToObject(json, "value", "unknown");
    }

    return json;
}

static cJSON *emit_literal(emit_t *emit, const hlir_t *hlir) {
    cJSON *literal = cJSON_CreateObject();
    cJSON_AddStringToObject(literal, "type", "literal");
    cJSON_AddItemToObject(literal, "value", emit_value(emit, hlir->literal));
    return literal;
}

static cJSON *emit_unimplemented(const hlir_t *hlir) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "unimplemented");
    cJSON_AddNumberToObject(json, "kind", hlir->type);
    return json;
}

static cJSON *lookup_value(emit_t *emit, const hlir_t *hlir) {
    size_t local = (size_t)map_get_ptr(emit->locals, hlir);
    if (local != 0) {
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "type", "local");
        cJSON_AddNumberToObject(json, "index", local);
        return json;
    }

    size_t global = (size_t)map_get_ptr(emit->globals, hlir);
    if (global != 0) {
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "type", "global");
        cJSON_AddNumberToObject(json, "index", global);
        return json;
    }

    return emit_unimplemented(hlir);
}

static cJSON *emit_expr(emit_t *emit, const hlir_t *hlir) {
    cJSON *result = NULL;

    switch (hlir->type) {
    case HLIR_LITERAL:
        result = emit_literal(emit, hlir);
        break;
    case HLIR_VALUE:
        result = lookup_value(emit, hlir);
        break;

    default:
        result = emit_unimplemented(hlir);
        break;
    }

    add_location(result, hlir->node);
    return result;
}

static cJSON *emit_global(emit_t *emit, const hlir_t *hlir) {
    cJSON *value = cJSON_CreateObject();
    cJSON_AddStringToObject(value, "kind", "value");
    cJSON_AddStringToObject(value, "name", hlir->name);
    cJSON_AddNumberToObject(value, "type", get_type(emit, hlir->of));
    cJSON_AddItemToObject(value, "value", emit_expr(emit, hlir->value));
    add_location(value, hlir->node);
    return value;
}

static cJSON *emit_import(emit_t *emit, const hlir_t *hlir) {
    cJSON *value = cJSON_CreateObject();

    switch (hlir->type) {
    case HLIR_IMPORT_FUNCTION:
        cJSON_AddStringToObject(value, "kind", "function");
        break;
    case HLIR_IMPORT_VALUE:
        cJSON_AddStringToObject(value, "kind", "value");
        break;
    default:
        cJSON_AddStringToObject(value, "kind", "unknown");
        break;
    }

    cJSON_AddStringToObject(value, "name", hlir->name);
    cJSON_AddNumberToObject(value, "type", get_type(emit, hlir->of));
    return value;
}

static cJSON *emit_compare(emit_t *emit, const hlir_t *hlir) {
    cJSON *result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "type", "compare");
    cJSON_AddStringToObject(result, "cmp", compare_name(hlir->compare));
    cJSON_AddItemToObject(result, "lhs", emit_expr(emit, hlir->lhs));
    cJSON_AddItemToObject(result, "rhs", emit_expr(emit, hlir->rhs));
    return result;
}

static cJSON *emit_stmt(emit_t *emit, const hlir_t *hlir);

static cJSON *emit_stmts(emit_t *emit, const hlir_t *hlir) {
    cJSON *stmts = cJSON_CreateObject();
    cJSON_AddStringToObject(stmts, "kind", "stmts");
    cJSON *items = cJSON_CreateArray();
    for (size_t i = 0; i < vector_len(hlir->stmts); i++) {
        const hlir_t *stmt = vector_get(hlir->stmts, i);
        cJSON *item = emit_stmt(emit, stmt);
        cJSON_AddItemToArray(items, item);
    }
    cJSON_AddItemToObject(stmts, "items", items);
    return stmts;
}

static cJSON *emit_branch(emit_t *emit, const hlir_t *hlir) {
    cJSON *branch = cJSON_CreateObject();
    cJSON_AddStringToObject(branch, "kind", "branch");
    cJSON_AddItemToObject(branch, "condition", emit_compare(emit, hlir->cond));
    cJSON_AddItemToObject(branch, "true", emit_stmt(emit, hlir->then));
    if (hlir->other != NULL) {
        cJSON_AddItemToObject(branch, "false", emit_stmt(emit, hlir->other));
    }
    return branch;
}

static cJSON *emit_stmt(emit_t *emit, const hlir_t *hlir) {
    cJSON *result = NULL;

    switch (hlir->type) {
    case HLIR_STMTS:
        result = emit_stmts(emit, hlir);
        break;
    case HLIR_BRANCH:
        result = emit_branch(emit, hlir);
        break;

    default:
        result = emit_unimplemented(hlir);
        break;
    }

    add_location(result, hlir->node);
    return result;
}

static cJSON *emit_function(emit_t *emit, const hlir_t *hlir) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "function");
    cJSON_AddStringToObject(json, "name", hlir->name);
    cJSON_AddNumberToObject(json, "type", get_type(emit, hlir->of));
    cJSON_AddItemToObject(json, "body", emit_stmt(emit, hlir->body));
    add_location(json, hlir->node);
    return json;
}

void emit_tree(const hlir_t *hlir) {
    size_t ntypes = vector_len(hlir->types);

    emit_t emit = {
        .types = optimal_map(ntypes)
    };

    cJSON *types = cJSON_CreateArray();
    cJSON_AddItemToArray(types, emit_type(&emit, 0, type_error("invalid type", NULL)));

    for (size_t i = 0; i < ntypes; i++) {
        const type_t *type = vector_get(hlir->types, i);
        cJSON *json = emit_type(&emit, i + 1, type);
        cJSON_AddItemToArray(types, json);
    }

    cJSON *imports = cJSON_CreateArray();
    size_t nimports = vector_len(hlir->imports);
    for (size_t i = 0; i < nimports; i++) {
        const hlir_t *it = vector_get(hlir->imports, i);
        cJSON *json = emit_import(&emit, it);
        cJSON_AddItemToArray(imports, json);
    }

    cJSON *globals = cJSON_CreateArray();
    size_t nglobals = vector_len(hlir->globals);
    for (size_t i = 0; i < nglobals; i++) {
        const hlir_t *global = vector_get(hlir->globals, i);
        cJSON *json = emit_global(&emit, global);
        cJSON_AddItemToArray(globals, json);
    }

    cJSON *functions = cJSON_CreateArray();
    size_t nfunctions = vector_len(hlir->defines);
    for (size_t i = 0; i < nfunctions; i++) {
        const hlir_t *function = vector_get(hlir->defines, i);
        cJSON *json = emit_function(&emit, function);
        cJSON_AddItemToArray(functions, json);
    }


    cJSON *json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "types", types);
    cJSON_AddItemToObject(json, "imports", imports);
    cJSON_AddItemToObject(json, "globals", globals);
    cJSON_AddItemToObject(json, "functions", functions);

    printf("%s\n", cJSON_Print(json));
}
