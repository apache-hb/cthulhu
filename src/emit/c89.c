#include "cthulhu/emit/emit.h"

static char *fmt_type_name(const hlir_t *type) {
    const char *base = nameof_hlir(type);
    base = replacestr(base, "_", "__");
    if (strcontains(base, "-") || strcontains(base, " ")) {
        base = replacestr(base, "-", "_");
        base = replacestr(base, " ", "_");
    }
    return format("type_%s", base);
}

static char *emit_signature(const hlir_t *type) {
    const char *args = "void";
    if (vector_len(type->params) > 0) {
        vector_t *params = VECTOR_MAP(type->params, fmt_type_name);
        args = strjoin(", ", params);
    }
    return format("%s(*%s)(%s%s)", fmt_type_name(type->result), fmt_type_name(type), args, type->variadic ? ", ..." : "");
}

static const char *sign_name(sign_t sign) {
    switch (sign) {
    case SIGN_UNSIGNED: return " unsigned";
    case SIGN_SIGNED: return " signed";
    default: return "";
    }
}

static const char *digit_name(digit_t digit) {
    switch (digit) {
    case DIGIT_CHAR: return "char";
    case DIGIT_SHORT: return "short";
    case DIGIT_INT: return "int";
    case DIGIT_LONG: return "long long";
    default: return "";
    }
}

static void emit_struct(const hlir_t *type) {
    printf("typedef struct %s {\n", fmt_type_name(type));
    for (size_t i = 0; i < vector_len(type->fields); i++) {
        const hlir_t *field = vector_get(type->fields, i);
        printf("    %s %s;\n", fmt_type_name(field), nameof_hlir(field));
    }
    printf("} %s;\n", fmt_type_name(type));
}

static void emit_type_decl(reports_t *reports, const hlir_t *type) {
    switch (type->type) {
    case HLIR_DIGIT:
        printf("typedef%s %s %s;\n", sign_name(type->sign), digit_name(type->width), fmt_type_name(type));
        break;
    case HLIR_BOOL:
        printf("typedef _Bool %s;\n", fmt_type_name(type));
        break;
    case HLIR_STRING:
        printf("typedef const char *%s;\n", fmt_type_name(type));
        break;
    case HLIR_VOID:
        printf("typedef void %s;\n", fmt_type_name(type));
        break;
    case HLIR_CLOSURE:
        printf("typedef %s;\n", emit_signature(type));
        break;
    case HLIR_ALIAS:
        printf("typedef %s %s;\n", fmt_type_name(type->alias), fmt_type_name(type));
        break;
    case HLIR_STRUCT:
        emit_struct(type);
        break;
    default:
        ctu_assert(reports, "invalid type %d", type->type);
        break;
    }
}

static char *get_type_params(const hlir_t *sig) {
    size_t len = vector_len(sig->params);
    if (len == 0) {
        return sig->variadic ? "..." : "void";
    }

    vector_t *types = VECTOR_MAP(sig->params, fmt_type_name);
    char *base = strjoin(", ", types);
    if (sig->variadic) {
        base = format("%s, ...", base);
    }
    return base;
}

static void emit_function_import(const hlir_t *hlir) {
    printf("extern %s %s(%s);\n", fmt_type_name(hlir->of->result), hlir->name, get_type_params(hlir->of));
}

static void emit_value_import(const hlir_t *hlir) {
    printf("extern %s %s;\n", fmt_type_name(hlir->of), hlir->name);
}

static void emit_import_decl(reports_t *reports, const hlir_t *hlir) {
    switch (hlir->type) {
    case HLIR_FUNCTION:
        return emit_function_import(hlir);
    case HLIR_VALUE:
        return emit_value_import(hlir);
    default:
        ctu_assert(reports, "invalid import type %d", hlir->type);
        break;
    }
}

static char *emit_expr(reports_t *reports, const hlir_t *hlir);

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
    case HLIR_VALUE:
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

static void fwd_global(const hlir_t *hlir) {
    printf("%s %s[1];\n", fmt_type_name(hlir->of), hlir->name);
}

static void emit_global(reports_t *reports, const hlir_t *hlir) {
    if (hlir->type != HLIR_VALUE) { return; }

    printf("%s %s[1] = { %s };\n", fmt_type_name(hlir->of), hlir->name, emit_expr(reports, hlir->value));
}

static void fwd_proc(const hlir_t *hlir) {
    printf("%s %s(%s);\n", fmt_type_name(hlir->of->result), hlir->name, get_type_params(hlir->of));
}

static void emit_proc(reports_t *reports, const hlir_t *hlir) {
    vector_t *locals = hlir->locals;

    printf("%s %s(%s) {\n", fmt_type_name(hlir->of->result), hlir->name, get_type_params(hlir->of));

    for (size_t i = 0; i < vector_len(locals); i++) {
        const hlir_t *local = vector_get(locals, i);
        printf("\t%s %s[1];\n", fmt_type_name(typeof_hlir(local)), local->name);
    }

    emit_stmt(reports, hlir->body);
    printf("}\n");
}

void c89_emit_tree(reports_t *reports, const hlir_t *hlir) {
    printf("// generated by %s\n", hlir->node->scan->path);
    printf("\n");

    size_t ntypes = vector_len(hlir->types);
    for (size_t i = 0; i < ntypes; i++) {
        const hlir_t *type = vector_get(hlir->types, i);
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
