#include "driver.h"

#include "ctu/util/report.h"

static void c_emit_digit(FILE *out, node_t *digit) {
    fprintf(out, "%s", mpz_get_str(NULL, 10, digit->digit));
}

static void c_emit_expr(FILE *out, node_t *expr) {
    switch (expr->kind) {
    case AST_DIGIT:
        c_emit_digit(out, expr);
        break;

    default:
        reportf(ERROR, expr->scan, expr->where, "c-emit-expr %d", expr->kind);
        break;
    }
}

static void c_emit_value(FILE *out, node_t *decl) {
    bool mutable = decl->mutable;
    const char *name = decl->name;
    node_t *value = decl->value;

    fprintf(out, "static%s int %s = ", 
        mutable ? "" : " const",
        name
    );

    c_emit_expr(out, value);

    fprintf(out, ";\n");
}

static void c_emit_decl(FILE *out, node_t *decl) {
    switch (decl->kind) {
    case AST_DECL_VALUE:
        c_emit_value(out, decl);
        break;

    default:
        reportf(ERROR, decl->scan, decl->where, "c-emit-decl %d", decl->kind);
        break;  
    }
}

void c_emit(FILE *out, node_t *node) {
    vector_t *decls = node->decls;

    for (size_t i = 0; i < vector_len(decls); i++) {
        node_t *decl = vector_get(decls, i);
        c_emit_decl(out, decl);
    }
}
