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

static char *emit_type(const type_t *type, const char *name) {
    switch (type->type) {
    case TY_INTEGER: return emit_digit(type->digit, name);
    case TY_VOID: return emit_void(name);
    default: return NULL;
    }
}

static char *emit_expr(lir_t *expr);

static char *emit_name(lir_t *id) {
    return ctu_strdup(id->name);
}

static char *emit_literal(mpz_t digit) {
    return mpz_get_str(NULL, 10, digit);
}

static const char *binary_op(binary_t binary) {
    switch (binary) {
    case BINARY_ADD: return "+";
    case BINARY_SUB: return "-";
    case BINARY_MUL: return "*";
    case BINARY_DIV: return "/";
    case BINARY_REM: return "%";

    case BINARY_EQ: return "==";
    case BINARY_NEQ: return "!=";
    case BINARY_LT: return "<";
    case BINARY_LTE: return "<=";
    case BINARY_GT: return ">";
    case BINARY_GTE: return ">=";

    default: return NULL;
    }
}

static char *emit_binary(lir_t *binary) {
    const char *op = binary_op(binary->binary);
    char *lhs = emit_expr(binary->lhs);
    char *rhs = emit_expr(binary->rhs);

    return format("%s %s %s", lhs, op, rhs);
}

static char *emit_expr(lir_t *expr) {
    switch (expr->leaf) {
    case LIR_VALUE: return emit_name(expr);
    case LIR_DIGIT: return emit_literal(expr->digit);
    case LIR_BINARY: return emit_binary(expr);
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
