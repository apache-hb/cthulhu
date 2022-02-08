#include "cthulhu/emit/emit.h"

static char *get_type(const type_t *type) {
    return format("type_%s", type->name);
}

static char *emit_signature(const type_t *type) {
    const char *args = "void";
    if (vector_len(type->params) > 0) {
        vector_t *params = VECTOR_MAP(type->params, get_type);
        args = strjoin(", ", params);
    }
    return format("%s(*type_%s)(%s%s)", get_type(type->result), type_get_name(type), args, type->variadic ? ", ..." : "");
}

static void emit_type_decl(reports_t *reports, const type_t *type) {
    switch (type->type) {
    case TYPE_INTEGER:
        printf("typedef int type_%s;\n", type->name);
        break;
    case TYPE_BOOLEAN:
        printf("typedef bool type_%s;\n", type->name);
        break;
    case TYPE_STRING:
        printf("typedef const char *type_%s;\n", type->name);
        break;
    case TYPE_VOID:
        printf("typedef void type_%s;\n", type->name);
        break;
    case TYPE_SIGNATURE:
        printf("typedef %s;\n", emit_signature(type));
        break;
    default:
        ctu_assert(reports, "invalid type");
        break;
    }
}

static char *get_type_params(const type_t *sig) {
    size_t len = vector_len(sig->params);
    if (len == 0) {
        return sig->variadic ? "..." : "void";
    }

    vector_t *types = VECTOR_MAP(sig->params, get_type);
    char *base = strjoin(", ", types);
    if (sig->variadic) {
        base = format("%s, ...", base);
    }
    return base;
}

static void emit_function_import(const hlir_t *hlir) {
    printf("extern %s %s(%s);\n", get_type(hlir->of->result), hlir->name, get_type_params(hlir->of));
}

static void emit_value_import(const hlir_t *hlir) {
    printf("extern %s %s;\n", get_type(hlir->of), hlir->name);
}

static void emit_import_decl(reports_t *reports, const hlir_t *hlir) {
    switch (hlir->type) {
    case HLIR_FUNCTION:
        return emit_function_import(hlir);
    case HLIR_VALUE:
        return emit_value_import(hlir);
    default:
        ctu_assert(reports, "invalid import type");
        break;
    }
}

static char *emit_expr(reports_t *reports, const hlir_t *hlir);

static char *emit_binary(reports_t *reports, const hlir_t *hlir) {
    char *lhs = emit_expr(reports, hlir->lhs);
    char *rhs = emit_expr(reports, hlir->rhs);

    return format("(%s %s %s)", lhs, binary_symbol(hlir->binary), rhs);
}

static char *emit_literal(reports_t *reports, const value_t *value) {
    if (type_is_boolean(value->type)) {
        return value->boolean ? "true" : "false";
    }

    if (type_is_integer(value->type)) {
        return mpz_get_str(NULL, 10, value->integer);
    }

    if (type_is_string(value->type)) {
        return format("\"%s\"", strnorm(value->string));
    }

    ctu_assert(reports, "invalid literal");
    return NULL;
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
    case HLIR_VALUE:
        return ctu_strdup(hlir->name);
    case HLIR_BINARY:
        return emit_binary(reports, hlir);
    case HLIR_NAME:
        return format("(%s[0])", emit_expr(reports, hlir->read));
    case HLIR_LITERAL:
        return emit_literal(reports, hlir->literal);
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

static void fwd_global(const hlir_t *hlir) {
    printf("%s %s[1];\n", get_type(hlir->of), hlir->name);
}

static void emit_global(reports_t *reports, const hlir_t *hlir) {
    printf("%s %s[1] = { %s };\n", get_type(hlir->of), hlir->name, emit_expr(reports, hlir->value));
}

static void fwd_proc(const hlir_t *hlir) {
    printf("%s %s(%s);\n", get_type(hlir->of->result), hlir->name, get_type_params(hlir->of));
}

static void emit_proc(reports_t *reports, const hlir_t *hlir) {
    vector_t *locals = hlir->locals;

    printf("%s %s(%s) {\n", get_type(hlir->of->result), hlir->name, get_type_params(hlir->of));

    for (size_t i = 0; i < vector_len(locals); i++) {
        const hlir_t *local = vector_get(locals, i);
        printf("\t%s %s[1];\n", get_type(local->of), local->name);
    }

    emit_stmt(reports, hlir->body);
    printf("}\n");
}

void c89_emit_tree(reports_t *reports, const hlir_t *hlir) {
    printf("// generated by %s\n", hlir->node->scan->path);
    printf("#include <stdbool.h>\n");
    printf("\n");

    size_t ntypes = vector_len(hlir->types);
    for (size_t i = 0; i < ntypes; i++) {
        const type_t *type = vector_get(hlir->types, i);
        emit_type_decl(reports, type);
    }

    size_t nglobals = vector_len(hlir->globals);
    size_t nprocs = vector_len(hlir->defines);

    for (size_t i = 0; i < nglobals; i++) {
        const hlir_t *import = vector_get(hlir->globals, i);
        if (!hlir_is_imported(import)) { continue; }

        emit_import_decl(reports, import);
    }

    for (size_t i = 0; i < nprocs; i++) {
        const hlir_t *import = vector_get(hlir->defines, i);
        if (!hlir_is_imported(import)) { continue; }

        emit_import_decl(reports, import);
    }

    for (size_t i = 0; i < nglobals; i++) {
        const hlir_t *global = vector_get(hlir->globals, i);
        if (hlir_is_imported(global)) { continue; }
        
        fwd_global(global);
    }

    for (size_t i = 0; i < nprocs; i++) {
        const hlir_t *proc = vector_get(hlir->defines, i);
        if (hlir_is_imported(proc)) { continue; }

        fwd_proc(proc);
    }

    for (size_t i = 0; i < nglobals; i++) {
        const hlir_t *global = vector_get(hlir->globals, i);
        if (hlir_is_imported(global)) { continue; }

        emit_global(reports, global);
    }
    
    for (size_t i = 0; i < nprocs; i++) {
        const hlir_t *proc = vector_get(hlir->defines, i);
        if (hlir_is_imported(proc)) { continue; }

        emit_proc(reports, proc);
    }
}
