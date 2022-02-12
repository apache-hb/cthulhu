#include "sema.h"

typedef enum {
    TAG_PROCS,
    TAG_VARS,

    TAG_MAX
} tag_t;

static size_t GLOBAL_SIZES[TAG_MAX] = {
    [TAG_PROCS] = MAP_MASSIVE,
    [TAG_VARS] = MAP_MASSIVE,
};

static size_t LOCAL_SIZES[TAG_MAX] = {
    [TAG_PROCS] = MAP_SMALL,
    [TAG_VARS] = MAP_SMALL,
};

static bool types_equal(const type_t *lhs, const type_t *rhs) {
    if (lhs == rhs) { return true; }

    if (lhs->type != rhs->type) { return false; }

    return false;
}

static const char *type_to_string(const type_t *type) {
    if (type->name != NULL) { return type->name; }

    return "???";
}

static context_t *get_context(scan_t *scan) {
    return scan_get(scan);
}

static sema_t *get_sema(scan_t *scan) {
    return get_context(scan)->sema;
}

static reports_t *get_reports(scan_t *scan) {
    return get_sema(scan)->reports;
}

static hlir_t *get_decl(scan_t *scan, tag_t tag, const char *name) {
    return sema_get(get_sema(scan), tag, name);
}

static void fwd_decl(scan_t *scan, tag_t tag, const char *name, hlir_t *decl) {
    hlir_t *other = get_decl(scan, tag, name);
    if (other != NULL && !types_equal(decl->of, other->of)) {
        message_t *id = report(get_reports(scan), ERROR, decl->node, "duplicate declaration of %s with different types", name);
        report_append(id, other->node, "previous declaration");
        report_note(id, "previously defined with `%s`\nredefinition with `%s`", type_to_string(other->of), type_to_string(decl->of));
        return;
    }

    sema_set(get_sema(scan), tag, name, decl);
}

static void add_decl(scan_t *scan, tag_t tag, const char *name, hlir_t *decl) {
    hlir_t *other = get_decl(scan, tag, name);
    if (other != NULL) {
        message_t *id = report(get_reports(scan), ERROR, decl->node, "duplicate declaration of %s", name);
        report_append(id, other->node, "previous declaration");
        return;
    }

    sema_set(get_sema(scan), tag, name, decl);
}

static hlir_t *sema_digit(ast_t *ast) {
    return hlir_literal(ast->node, value_digit(get_digit(SIGN_DEFAULT, DIGIT_INT), ast->digit));
}

static hlir_t *sema_expr(sema_t *sema, ast_t *ast) {
    UNUSED(sema);

    switch (ast->type) {
    case AST_DIGIT: return sema_digit(ast);
    default: 
        ctu_assert(sema->reports, "sema-expr %d", ast->type);
        return NULL;
    }
}

context_t *new_context(reports_t *reports) {
    context_t *ctx = ctu_malloc(sizeof(context_t));
    ctx->path = NULL;
    ctx->current = NULL;
    ctx->default_int = get_digit(SIGN_DEFAULT, DIGIT_INT);
    ctx->sema = sema_new(NULL, reports, TAG_MAX, GLOBAL_SIZES);
    return ctx;
}

void set_current_type(scan_t *scan, type_t *type) {
    context_t *ctx = get_context(scan);
    ctx->current = type;
}

type_t *get_current_type(scan_t *scan) {
    context_t *ctx = get_context(scan);
    return ctx->current;
}

type_t *default_int(scan_t *scan, const node_t *node) {
    context_t *ctx = get_context(scan);
    ctx->current = ctx->default_int;

    report(get_reports(scan), WARNING, node, "default int is deprecated, use an explicit type instead");
    return ctx->default_int;
}

vardecl_t *new_vardecl(scan_t *scan, where_t where, type_t *type, char *name, ast_t *init) {
    vardecl_t *decl = ctu_malloc(sizeof(vardecl_t));
    decl->name = name;
    decl->type = type;
    decl->node = node_new(scan, where);
    decl->init = init;
    return decl;
}

void cc_module(scan_t *scan, vector_t *path) {
    context_t *ctx = scan_get(scan);
    ctx->path = strjoin(".", path);
}

void cc_vardecl(scan_t *scan, hlir_linkage_t storage, vector_t *decls) {
    size_t len = vector_len(decls);
    hlir_attributes_t *attribs = hlir_new_attributes(storage);

    for (size_t i = 0; i < len; i++) {
        vardecl_t *decl = vector_get(decls, i);

        hlir_t *hlir = hlir_new_value(decl->node, decl->name, decl->type);
        hlir_set_attributes(hlir, attribs);

        if (decl->init == NULL) {
            fwd_decl(scan, TAG_VARS, decl->name, hlir);
        } else {
            hlir_t *init = sema_expr(get_sema(scan), decl->init);
            hlir_build_value(hlir, init);
            add_decl(scan, TAG_VARS, decl->name, hlir);
        }
    }
}

void cc_finish(scan_t *scan, where_t where) {
    context_t *ctx = get_context(scan);

    hlir_t *hlir = hlir_new_module(node_new(scan, where), ctx->path);

    vector_t *vars = map_values(sema_tag(get_sema(scan), TAG_VARS));

    // collect up the default int types
    vector_t *types = vector_new(SIGN_TOTAL * DIGIT_TOTAL);
    for (int sign = 0; sign < SIGN_TOTAL; sign++) {
        for (int digit = 0; digit < DIGIT_TOTAL; digit++) {
            type_t *type = get_digit(sign, digit);
            vector_push(&types, type);
        }
    }

    hlir_build_module(hlir, vars, vector_of(0), types);

    scan_export(scan, hlir);
}
