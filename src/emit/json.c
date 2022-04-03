#include "cthulhu/emit/emit.h"

#include "cJSON.h"

typedef struct {
    reports_t *reports;
    map_t *types; // map of type to its index
    map_t *locals; // map of local to its index
    map_t *globals; // map of global to its index
    map_t *functions;
} emit_t;

static const char *metatype_to_string(emit_t *emit, const hlir_t *type) {
    switch (type->type) {
    case HLIR_BOOL: return "boolean";
    case HLIR_DIGIT: return "integer";
    case HLIR_STRING: return "string";
    case HLIR_VOID: return "void";
    case HLIR_CLOSURE: return "signature";
    case HLIR_ERROR: return "error";
    default: 
        ctu_assert(emit->reports, "unknown metatype");
        return "unknown";
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

static size_t get_type(emit_t *emit, const hlir_t *type) {
    return (size_t)map_get_ptr(emit->types, type);
}

static const char *SIGN_STR[SIGN_TOTAL] = {
    [SIGN_UNSIGNED] = "unsigned",
    [SIGN_SIGNED] = "signed",
    [SIGN_DEFAULT] = "default",
};

static const char *WIDTH_STR[DIGIT_TOTAL] = {
    [DIGIT_CHAR] = "char",
    [DIGIT_SHORT] = "short",
    [DIGIT_INT] = "int",
    [DIGIT_LONG] = "long",
};

static cJSON *emit_type(emit_t *emit, size_t idx, const hlir_t *hlir) {
    map_set_ptr(emit->types, hlir, (void*)(uintptr_t)idx);

    cJSON *type = cJSON_CreateObject();
    cJSON_AddStringToObject(type, "type", metatype_to_string(emit, hlir));
    cJSON_AddStringToObject(type, "name", hlir->name);

    if (hlir_is(hlir, HLIR_CLOSURE)) {
        cJSON_AddNumberToObject(type, "result", get_type(emit, closure_result(hlir)));
        cJSON *params = cJSON_CreateArray();
        vector_t *vec = closure_params(hlir);
        for (size_t i = 0; i < vector_len(vec); i++) {
            const hlir_t *param = vector_get(vec, i);
            cJSON_AddItemToArray(params, cJSON_CreateNumber(get_type(emit, param)));
        }
        cJSON_AddItemToObject(type, "params", params);
        cJSON_AddBoolToObject(type, "variadic", closure_variadic(hlir));
    } else if (hlir_is(hlir, HLIR_DIGIT)) {
        cJSON_AddStringToObject(type, "sign", SIGN_STR[hlir->sign]);
        cJSON_AddStringToObject(type, "width", WIDTH_STR[hlir->width]);
    }

    add_location(type, hlir->node);

    return type;
}

static const char *linkage_name(reports_t *reports, hlir_linkage_t linkage) {
    switch (linkage) {
    case LINK_EXPORTED: return "exported";
    case LINK_IMPORTED: return "imported";
    case LINK_INTERNAL: return "internal";
    default: 
        ctu_assert(reports, "unknown linkage %d", linkage);
        return "unknown";
    }
}

static cJSON *emit_attribs(reports_t *reports, const hlir_attributes_t *attribs) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "linkage", linkage_name(reports, attribs->linkage));
    return json;
}

static cJSON *emit_digit(emit_t *emit, const hlir_t *hlir) {
    cJSON *literal = cJSON_CreateObject();
    cJSON_AddStringToObject(literal, "kind", "int-literal");
    cJSON_AddNumberToObject(literal, "type", get_type(emit, typeof_hlir(hlir)));
    cJSON_AddNumberToObject(literal, "value", mpz_get_si(hlir->digit));
    return literal;
}

static cJSON *emit_bool(emit_t *emit, const hlir_t *hlir) {
    cJSON *literal = cJSON_CreateObject();
    cJSON_AddStringToObject(literal, "kind", "bool-literal");
    cJSON_AddNumberToObject(literal, "type", get_type(emit, typeof_hlir(hlir)));
    cJSON_AddBoolToObject(literal, "value", hlir->boolean);
    return literal;
}

static cJSON *emit_string(emit_t *emit, const hlir_t *hlir) {
    cJSON *literal = cJSON_CreateObject();
    cJSON_AddStringToObject(literal, "kind", "string-literal");
    cJSON_AddNumberToObject(literal, "type", get_type(emit, typeof_hlir(hlir)));
    cJSON_AddStringToObject(literal, "value", hlir->string);
    return literal;
}

static cJSON *emit_unimplemented(const hlir_t *hlir) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "kind", "unimplemented");
    cJSON_AddNumberToObject(json, "type", hlir->type);
    return json;
}

static cJSON *lookup_value(emit_t *emit, const hlir_t *hlir) {
    size_t global = (size_t)map_get_ptr(emit->globals, hlir);
    if (global != 0) {
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "kind", "global");
        cJSON_AddNumberToObject(json, "index", global);
        return json;
    }

    size_t local = (size_t)map_get_ptr(emit->locals, hlir);
    if (local != 0) {
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "kind", "local");
        cJSON_AddNumberToObject(json, "index", local);
        return json;
    }

    ctu_assert(emit->reports, "failed value lookup");
    return emit_unimplemented(hlir);
}

static cJSON *emit_expr(emit_t *emit, const hlir_t *hlir);

static cJSON *emit_binary(emit_t *emit, const hlir_t *hlir) {
    cJSON *binary = cJSON_CreateObject();
    cJSON_AddStringToObject(binary, "kind", "binary");
    cJSON_AddStringToObject(binary, "op", binary_name(hlir->binary));
    cJSON_AddItemToObject(binary, "lhs", emit_expr(emit, hlir->lhs));
    cJSON_AddItemToObject(binary, "rhs", emit_expr(emit, hlir->rhs));
    return binary;
}

static cJSON *emit_name(emit_t *emit, const hlir_t *hlir) {
    cJSON *name = cJSON_CreateObject();
    cJSON_AddStringToObject(name, "kind", "name");
    cJSON_AddItemToObject(name, "read", emit_expr(emit, hlir->read));
    return name;
}

static cJSON *emit_call(emit_t *emit, const hlir_t *hlir) {
    cJSON *call = cJSON_CreateObject();
    cJSON_AddStringToObject(call, "kind", "call");
    cJSON_AddItemToObject(call, "func", emit_expr(emit, hlir->call));
    cJSON *args = cJSON_CreateArray();
    for (size_t i = 0; i < vector_len(hlir->args); i++) {
        cJSON_AddItemToArray(args, emit_expr(emit, vector_get(hlir->args, i)));
    }
    cJSON_AddItemToObject(call, "args", args);
    return call;
}

static cJSON *lookup_function(emit_t *emit, const hlir_t *hlir) {
    size_t idx = (size_t)map_get_ptr(emit->functions, hlir);
    if (idx != 0) {
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "kind", "function");
        cJSON_AddNumberToObject(json, "index", idx);
        return json;
    }

    ctu_assert(emit->reports, "failed function lookup for %s", hlir->name);
    return emit_unimplemented(hlir);
}

static cJSON *emit_expr(emit_t *emit, const hlir_t *hlir) {
    cJSON *result = NULL;

    switch (hlir->type) {
    case HLIR_DIGIT_LITERAL:
        result = emit_digit(emit, hlir);
        break;
    case HLIR_BOOL_LITERAL:
        result = emit_bool(emit, hlir);
        break;
    case HLIR_STRING_LITERAL:
        result = emit_string(emit, hlir);
        break;
    case HLIR_GLOBAL:
        result = lookup_value(emit, hlir);
        break;
    case HLIR_FUNCTION:
        result = lookup_function(emit, hlir);
        break;
    case HLIR_BINARY:
        result = emit_binary(emit, hlir);
        break;
    case HLIR_NAME:
        result = emit_name(emit, hlir);
        break;
    case HLIR_CALL:
        result = emit_call(emit, hlir);
        break;
    default:
        ctu_assert(emit->reports, "unknown expr type");
        result = emit_unimplemented(hlir);
        break;
    }

    add_location(result, hlir->node);
    return result;
}

static cJSON *emit_global(emit_t *emit, map_t *map, size_t index, const hlir_t *hlir) {
    map_set_ptr(map, hlir, (void*)(uintptr_t)index);

    cJSON *value = cJSON_CreateObject();
    cJSON_AddStringToObject(value, "kind", "value");
    cJSON_AddStringToObject(value, "name", hlir->name);
    cJSON_AddNumberToObject(value, "type", get_type(emit, typeof_hlir(hlir)));
    cJSON_AddItemToObject(value, "attribs", emit_attribs(emit->reports, hlir->attributes));

    if (hlir->value != NULL) {
        cJSON_AddItemToObject(value, "value", emit_expr(emit, hlir->value));
    }
    add_location(value, hlir->node);
    return value;
}

static cJSON *emit_compare(emit_t *emit, const hlir_t *hlir) {
    cJSON *result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "kind", "compare");
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

static cJSON *emit_assign(emit_t *emit, const hlir_t *hlir) {
    cJSON *assign = cJSON_CreateObject();
    cJSON_AddStringToObject(assign, "kind", "assign");
    cJSON_AddItemToObject(assign, "dst", emit_expr(emit, hlir->dst));
    cJSON_AddItemToObject(assign, "src", emit_expr(emit, hlir->src));
    return assign;
}

static cJSON *emit_loop(emit_t *emit, const hlir_t *hlir) {
    cJSON *loop = cJSON_CreateObject();
    cJSON_AddStringToObject(loop, "kind", "loop");
    cJSON_AddItemToObject(loop, "cond", emit_compare(emit, hlir->cond));
    cJSON_AddItemToObject(loop, "then", emit_stmt(emit, hlir->then));
    if (hlir->other != NULL) {
        cJSON_AddItemToObject(loop, "else", emit_stmt(emit, hlir->other));
    }
    return loop;
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
    case HLIR_LOOP:
        result = emit_loop(emit, hlir);
        break;
    case HLIR_ASSIGN:
        result = emit_assign(emit, hlir);
        break;
    
    case HLIR_CALL:
        return emit_expr(emit, hlir);

    default:
        ctu_assert(emit->reports, "unknown stmt type");
        result = emit_unimplemented(hlir);
        break;
    }

    add_location(result, hlir->node);
    return result;
}

static cJSON *emit_function(emit_t *emit, size_t idx, const hlir_t *hlir) {
    map_set_ptr(emit->functions, hlir, (void*)(uintptr_t)idx);

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "kind", "function");
    cJSON_AddStringToObject(json, "name", hlir->name);
    cJSON_AddNumberToObject(json, "type", get_type(emit, typeof_hlir(hlir)));
    cJSON_AddItemToObject(json, "attribs", emit_attribs(emit->reports, hlir->attributes));

    size_t nlocals = vector_len(hlir->locals);
    cJSON *locals = cJSON_CreateArray();
    cJSON_AddItemToArray(locals, cJSON_CreateNull());
    for (size_t i = 0; i < nlocals; i++) {
        const hlir_t *local = vector_get(hlir->locals, i);
        cJSON *item = emit_global(emit, emit->locals, i + 1, local);
        cJSON_AddItemToArray(locals, item);
    }

    if (!hlir_is_imported(hlir)) {
        cJSON_AddItemToObject(json, "body", emit_stmt(emit, hlir->body));
    }
    
    add_location(json, hlir->node);
    return json;
}

void json_emit_tree(reports_t *reports, const hlir_t *hlir) {
    size_t ntypes = vector_len(hlir->types);
    size_t nglobals = vector_len(hlir->globals);
    size_t nfunctions = vector_len(hlir->functions);

    emit_t emit = {
        .reports = reports,
        .types = optimal_map(ntypes),
        .globals = optimal_map(nglobals),
        .functions = optimal_map(nfunctions)
    };

    cJSON *types = cJSON_CreateArray();
    cJSON_AddItemToArray(types, cJSON_CreateNull());

    for (size_t i = 0; i < ntypes; i++) {
        const hlir_t *type = vector_get(hlir->types, i);
        cJSON *json = emit_type(&emit, i + 1, type);
        cJSON_AddItemToArray(types, json);
    }

    cJSON *globals = cJSON_CreateArray();
    cJSON_AddItemToArray(globals, cJSON_CreateNull());

    for (size_t i = 0; i < nglobals; i++) {
        const hlir_t *global = vector_get(hlir->globals, i);
        cJSON *json = emit_global(&emit, emit.globals, i + 1, global);
        cJSON_AddItemToArray(globals, json);
    }

    cJSON *functions = cJSON_CreateArray();
    cJSON_AddItemToArray(functions, cJSON_CreateNull());

    for (size_t i = 0; i < nfunctions; i++) {
        const hlir_t *function = vector_get(hlir->functions, i);
        map_set_ptr(emit.functions, function, (void*)(uintptr_t)i + 1);
    }

    for (size_t i = 0; i < nfunctions; i++) {
        const hlir_t *function = vector_get(hlir->functions, i);
        size_t nlocals = vector_len(function->locals);
        emit.locals = optimal_map(nlocals);

        cJSON *json = emit_function(&emit, i + 1, function);
        cJSON_AddItemToArray(functions, json);
    }

    cJSON *json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "types", types);
    cJSON_AddItemToObject(json, "globals", globals);
    cJSON_AddItemToObject(json, "functions", functions);

    printf("%s\n", cJSON_Print(json));
}
