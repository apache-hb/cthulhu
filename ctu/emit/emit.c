#include "emit.h"

#include "ctu/util/str.h"

static const char *str_digit(bool sign, int_t kind) {
    switch (kind) {
    case TY_CHAR: return sign ? "char" : "unsigned char";
    case TY_SHORT: return sign ? "short" : "unsigned short";
    case TY_INT: return sign ? "int" : "unsigned int";
    case TY_LONG: return sign ? "long" : "unsigned long";
    default: return NULL;
    }
}

static char *emit_digit(digit_t digit, const char *name) {
    const char *num = str_digit(digit.sign, digit.kind);
    
    if (name == NULL) {
        return ctu_strdup(num);
    } else {
        return format("%s %s", num, name);
    }
}

static char *emit_void(const char *name) {
    if (name == NULL) {
        return ctu_strdup("void");
    } else {
        return format("void %s", name);
    }
}

static char *emit_type(type_t *type, const char *name) {
    switch (type->type) {
    case TY_DIGIT: return emit_digit(type->digit, name);
    case TY_VOID: return emit_void(name);
    default: return NULL;
    }
}

static char *emit_name(lir_t *name) {
    lir_t *it = name->id;
    return ctu_strdup(it->name);
}

static char *emit_literal(mpz_t digit) {
    return mpz_get_str(NULL, 10, digit);
}

static char *emit_expr(lir_t *expr) {
    switch (expr->leaf) {
    case LIR_NAME: return emit_name(expr->id);
    case LIR_DIGIT: return emit_literal(expr->digit);
    default: return NULL;
    }
}

static char *emit_value(lir_t *value) {
    char *front = emit_type(value->type, value->name);

    if (value->init) {
        char *init = emit_expr(value->init);

        return format("%s = %s", front, init);
    }

    return front;
}

void emit_c(FILE *out, lir_t *mod) {
    vector_t *vars = mod->vars;
    size_t len = vector_len(vars);

    for (size_t i = 0; i < len; i++) {
        lir_t *var = vector_get(vars, i);
        char *val = emit_value(var);
        fprintf(out, "%s;\n", val);
    }
}
