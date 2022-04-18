#include "cthulhu/emit/emit.h"
#include "cthulhu/hlir/query.h"

static char *fmt_type_name(const hlir_t *type) {
    const char *base = get_hlir_name(type);
    base = replacestr(base, "_", "__");
    if (strcontains(base, "-") || strcontains(base, " ")) {
        base = replacestr(base, "-", "_");
        base = replacestr(base, " ", "_");
    }
    return format("type_%s", base);
}

static const char *emit_type(reports_t *reports, const hlir_t *type, const char *name);
static char *emit_expr(reports_t *reports, const hlir_t *hlir);

static const char *emit_bool(const char *name) {
    return name != NULL ? format("_Bool %s", name) : "_Bool";
}

static const char *emit_void(const char *name) {
    return name != NULL ? format("void %s", name) : "void";
}

static const char *emit_ptr(reports_t *reports, const hlir_t *type, const char *name) {
    const char *base = emit_type(reports, type->ptr, NULL);
    return name != NULL ? format("%s *%s", base, name) : base;
}

static const char *emit_array(reports_t *reports, const hlir_t *type, const char *name) {
    const char *base = emit_type(reports, type->element, NULL);
    char *length = emit_expr(reports, type->length);

    return name != NULL ? format("%s %s[%s]", base, name, length) 
                        : format("%s[%s]", base, length);
}

static const char *SIGNS[SIGN_TOTAL] = {
    [SIGN_DEFAULT] = "",
    [SIGN_SIGNED] = "signed ",
    [SIGN_UNSIGNED] = "unsigned "
};

static const char *DIGITS[DIGIT_TOTAL] = {
    [DIGIT_CHAR] = "char",
    [DIGIT_SHORT] = "short",
    [DIGIT_INT] = "int",
    [DIGIT_LONG] = "long",
    [DIGIT_SIZE] = "size_t",
    [DIGIT_PTR] = "intptr_t"
};

static char *emit_digit(const hlir_t *type, const char *name) {
    char *digit = format("%s%s", SIGNS[type->sign], DIGITS[type->width]);
    return name != NULL ? format("%s %s", digit, name) : digit;
}

static const char *emit_string(const char *name) {
    return name != NULL ? format("const char *%s", name) : "const char *";
}

static char *emit_struct(const hlir_t *type, const char *name) {
    char *base = format("struct %s", fmt_type_name(type));
    return name != NULL ? format("%s %s", base, name) : base;
}

static char *emit_union(const hlir_t *type, const char *name) {
    char *base = format("union %s", fmt_type_name(type));
    return name != NULL ? format("%s %s", base, name) : base;
}

static char *emit_closure(reports_t *reports, const hlir_t *type, const char *name) {
    const char *result = emit_type(reports, type->result, NULL);

    const char *args = "void";
    size_t len = vector_len(type->params);
    if (len > 0) {
        vector_t *params = vector_of(len);
        for (size_t i = 0; i < len; i++) {
            const hlir_t *param = vector_get(type->params, i);
            const char *kind = emit_type(reports, param, NULL);
            vector_set(params, i, (void*)kind);
        }
        args = strjoin(", ", params);
    }

    return name != NULL ? format("%s (*%s)(%s)", result, name, args) 
                        : format("%s (*)(%s)", result, args);
}

static const char *emit_type(reports_t *reports, const hlir_t *type, const char *name) {
    hlir_kind_t kind = get_hlir_kind(type);
    switch (kind) {
    case HLIR_BOOL: return emit_bool(name);
    case HLIR_VOID: return emit_void(name);
    case HLIR_DIGIT: return emit_digit(type, name);
    case HLIR_STRING: return emit_string(name);

    case HLIR_POINTER: return emit_ptr(reports, type, name);
    case HLIR_ARRAY: return emit_array(reports, type, name);

    case HLIR_CLOSURE: return emit_closure(reports, type, name);

    case HLIR_STRUCT: return emit_struct(type, name);
    case HLIR_UNION: return emit_union(type, name);

    case HLIR_ALIAS: return emit_type(reports, type->alias, name);

    default:
        ctu_assert(reports, "cannot emit %s as a type", hlir_kind_to_string(kind));
        return "error";
    }
}

static void emit_aggregate_decl(reports_t *reports, const hlir_t *type, const char *kind) {
    const char *name = fmt_type_name(type);
    printf("%s %s {\n", kind, name);
    for (size_t i = 0; i < vector_len(type->fields); i++) {
        const hlir_t *field = vector_get(type->fields, i);
        printf("    %s;\n", emit_type(reports, get_hlir_type(field), get_hlir_name(field)));
    }
    printf("};\n");
}

static void emit_type_decl(reports_t *reports, const hlir_t *type) {
    switch (type->type) {
    case HLIR_STRUCT:
        emit_aggregate_decl(reports, type, "struct");
        break;
        
    case HLIR_UNION:
        emit_aggregate_decl(reports, type, "union");
        break;

    default:
        ctu_assert(reports, "invalid type %d", type->type);
        break;
    }
}

static char *get_type_params(reports_t *reports, const hlir_t *sig) {
    size_t len = vector_len(sig->params);
    if (len == 0) {
        return sig->variadic ? "..." : "void";
    }

    vector_t *types = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        const hlir_t *param = vector_get(sig->params, i);
        const char *type = emit_type(reports, param, NULL);
        vector_set(types, i, (char*)type);
    }

    char *base = strjoin(", ", types);
    if (sig->variadic) {
        base = format("%s, ...", base);
    }
    return base;
}

static void emit_function_import(reports_t *reports, const hlir_t *hlir) {
    const char *type = emit_type(reports, hlir->result, get_hlir_name(hlir));
    printf("extern %s(%s);\n", type, get_type_params(reports, hlir));
}

static void emit_value_import(reports_t *reports, const hlir_t *hlir) {
    const char *type = emit_type(reports, hlir, get_hlir_name(hlir));
    printf("extern %s;\n", type);
}

static void emit_import_decl(reports_t *reports, const hlir_t *hlir) {
    switch (hlir->type) {
    case HLIR_FUNCTION:
        emit_function_import(reports, hlir);
        break;
    case HLIR_GLOBAL:
        emit_value_import(reports, hlir);
        break;
    default:
        ctu_assert(reports, "invalid import type %d", hlir->type);
        break;
    }
}

static char *emit_binary(reports_t *reports, const hlir_t *hlir) {
    char *lhs = emit_expr(reports, hlir->lhs);
    char *rhs = emit_expr(reports, hlir->rhs);

    return format("(%s %s %s)", lhs, binary_symbol(hlir->binary), rhs);
}

static char *emit_call(reports_t *reports, const hlir_t *hlir) {
    char *call = emit_expr(reports, hlir->call);
    size_t len = vector_len(hlir->args);
    if (len == 0) {
        return format("%s()", call);
    }

    vector_t *args = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        vector_set(args, i, emit_expr(reports, vector_get(hlir->args, i)));
    }
    char *joined = strjoin(", ", args);

    return format("%s(%s)", call, joined);
}

static char *emit_expr(reports_t *reports, const hlir_t *hlir) {
    switch (hlir->type) {
    case HLIR_FUNCTION:
    case HLIR_GLOBAL: case HLIR_LOCAL:
        return ctu_strdup(hlir->name);
    case HLIR_BINARY:
        return emit_binary(reports, hlir);
    case HLIR_NAME:
        return format("(%s[0])", emit_expr(reports, hlir->read));
    case HLIR_DIGIT_LITERAL:
        return mpz_get_str(NULL, 10, hlir->digit);
    case HLIR_BOOL_LITERAL:
        return hlir->boolean ? "1" : "0";
    case HLIR_STRING_LITERAL:
        return format("\"%s\"", strnorm(hlir->string));
    case HLIR_CALL:
        return emit_call(reports, hlir);
    default:
        ctu_assert(reports, "invalid expression type");
        return NULL;
    }
}

static char *emit_compare(reports_t *reports, const hlir_t *hlir) {
    char *lhs = emit_expr(reports, hlir->lhs);
    char *rhs = emit_expr(reports, hlir->rhs);

    return format("(%s %s %s)", lhs, compare_symbol(hlir->compare), rhs);
}

static char *emit_condition(reports_t *reports, const hlir_t *hlir) {
    switch (hlir->type) {
    case HLIR_COMPARE:
        return emit_compare(reports, hlir);
    default:
        ctu_assert(reports, "invalid condition type");
        return NULL;
    }
}

static void emit_stmt(reports_t *reports, const hlir_t *hlir);

static void emit_stmts(reports_t *reports, const hlir_t *hlir) {
    for (size_t i = 0; i < vector_len(hlir->stmts); i++) {
        emit_stmt(reports, vector_get(hlir->stmts, i));
    }
}

static void emit_assign(reports_t *reports, const hlir_t *hlir) {
    char *dst = emit_expr(reports, hlir->dst);
    char *src = emit_expr(reports, hlir->src);

    printf("\t%s[0] = %s;\n", dst, src);
}

static void emit_branch(reports_t *reports, const hlir_t *hlir) {
    char *cond = emit_condition(reports, hlir->cond);
    printf("\tif (%s) {\n", cond);
    emit_stmt(reports, hlir->then);
    printf("\t}\n");
}

static void emit_loop(reports_t *reports, const hlir_t *hlir) {
    printf("\twhile (%s) {\n", emit_condition(reports, hlir->cond));
    emit_stmt(reports, hlir->then);
    printf("\t}\n");
}

static void emit_stmt(reports_t *reports, const hlir_t *hlir) {
    switch (hlir->type) {
    case HLIR_STMTS: 
        emit_stmts(reports, hlir); 
        break;
    case HLIR_ASSIGN:
        emit_assign(reports, hlir);
        break;
    case HLIR_BRANCH:
        emit_branch(reports, hlir);
        break;
    case HLIR_LOOP:
        emit_loop(reports, hlir);
        break;
    case HLIR_CALL:
        printf("\t%s;\n", emit_expr(reports, hlir));
        break;
    default:
        ctu_assert(reports, "invalid statement type");
        break;
    }
}

static const char *emit_hlir_type(reports_t *reports, const hlir_t *hlir) {
    return emit_type(reports, get_hlir_type(hlir), get_hlir_name(hlir));
}

static void fwd_global(reports_t *reports, const hlir_t *hlir) {
    printf("%s[1];\n", emit_hlir_type(reports, hlir));
}

static void emit_global(reports_t *reports, const hlir_t *hlir) {
    if (hlir->type != HLIR_GLOBAL) { return; }

    printf("%s[1] = { %s };\n", emit_hlir_type(reports, hlir), emit_expr(reports, hlir->value));
}

static void fwd_proc(reports_t *reports, const hlir_t *hlir) {
    const char *type = emit_type(reports, hlir->result, get_hlir_name(hlir));
    const char *params = get_type_params(reports, hlir);

    printf("%s(%s);\n", type, params);
}

static void emit_proc(reports_t *reports, const hlir_t *hlir) {
    vector_t *locals = hlir->locals;

    const char *type = emit_type(reports, hlir->result, get_hlir_name(hlir));
    const char *params = get_type_params(reports, hlir);
    printf("%s(%s) {\n", type, params);

    for (size_t i = 0; i < vector_len(locals); i++) {
        const hlir_t *local = vector_get(locals, i);
        printf("\t%s[1];\n", emit_hlir_type(reports, local));
    }

    emit_stmt(reports, hlir->body);
    printf("}\n");
}

static void visit_type(reports_t *reports, vector_t **result, const hlir_t *type) {
    if (vector_find(*result, type) != SIZE_MAX) { return; }

    switch (type->type) {
    case HLIR_VOID: case HLIR_DIGIT:
    case HLIR_BOOL: case HLIR_STRING:
    case HLIR_POINTER: case HLIR_ARRAY:
        break;

    case HLIR_ALIAS:
        visit_type(reports, result, type->alias);
        break;

    case HLIR_FIELD:
        visit_type(reports, result, get_hlir_type(type));
        break;
    
    case HLIR_CLOSURE:
        visit_type(reports, result, type->result);
        for (size_t i = 0; i < vector_len(type->params); i++) {
            visit_type(reports, result, vector_get(type->params, i));
        }
        break;
    
    case HLIR_STRUCT: case HLIR_UNION:
        for (size_t i = 0; i < vector_len(type->fields); i++) {
            visit_type(reports, result, vector_get(type->fields, i));
        }
        vector_push(result, (void*)type);
        break;

    default:
        ctu_assert(reports, "invalid type to sort %d", type->type);
    }
}

static vector_t *sorted_types(reports_t *reports, vector_t *types) {
    size_t len = vector_len(types);
    vector_t *result = vector_new(len);

    for (size_t i = 0; i < len; i++) {
        visit_type(reports, &result, vector_get(types, i));
    }

    return result;
}

void c89_emit_tree(reports_t *reports, const hlir_t *hlir) {
    printf("// generated by %s\n", hlir->node->scan->path);
    printf("\n");

    // this actually sorts and also removes types that 
    // we dont need to declare in our C code
    vector_t *types = sorted_types(reports, hlir->types);
    size_t ntypes = vector_len(types);
    for (size_t i = 0; i < ntypes; i++) {
        const hlir_t *type = vector_get(types, i);
        emit_type_decl(reports, type);
    }

    size_t nglobals = vector_len(hlir->globals);
    size_t nprocs = vector_len(hlir->functions);

    for (size_t i = 0; i < nglobals; i++) {
        const hlir_t *import = vector_get(hlir->globals, i);
        if (!hlir_is_imported(import)) { continue; }

        emit_import_decl(reports, import);
    }

    for (size_t i = 0; i < nprocs; i++) {
        const hlir_t *import = vector_get(hlir->functions, i);
        if (!hlir_is_imported(import)) { continue; }

        emit_import_decl(reports, import);
    }

    for (size_t i = 0; i < nglobals; i++) {
        const hlir_t *global = vector_get(hlir->globals, i);
        if (hlir_is_imported(global)) { continue; }
        
        fwd_global(reports, global);
    }

    for (size_t i = 0; i < nprocs; i++) {
        const hlir_t *proc = vector_get(hlir->functions, i);
        if (hlir_is_imported(proc)) { continue; }

        fwd_proc(reports, proc);
    }

    for (size_t i = 0; i < nglobals; i++) {
        const hlir_t *global = vector_get(hlir->globals, i);
        if (hlir_is_imported(global)) { continue; }

        emit_global(reports, global);
    }
    
    for (size_t i = 0; i < nprocs; i++) {
        const hlir_t *proc = vector_get(hlir->functions, i);
        if (hlir_is_imported(proc)) { continue; }

        emit_proc(reports, proc);
    }
}
