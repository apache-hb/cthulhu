#include "lir.h"

#include "ctu/util/str.h"

static size_t depth = 0;

static const char *leaf_name(leaf_t leaf) {
    switch (leaf) {
    case LIR_DIGIT: return "digit";
    case LIR_NAME: return "name";
    case LIR_BINARY: return "binary";
    case LIR_UNARY: return "unary";
    case LIR_CALL: return "call";
    case LIR_ASSIGN: return "assign";
    case LIR_WHILE: return "while";
    case LIR_BRANCH: return "branch";
    case LIR_STMTS: return "stmts";
    case LIR_VALUE: return "value";
    case LIR_DEFINE: return "define";
    case LIR_MODULE: return "module";
    case LIR_FORWARD: return "forward";
    case LIR_POISON: return "poison";
    default: return "???";
    }
}

static char *print_digit(const lir_t *lir) {
    return mpz_get_str(NULL, 10, lir->digit);
}

static char *print_name(const lir_t *lir) {
    return print_lir(lir->it);
}

static char *print_binary(const lir_t *lir) {
    char *lhs = print_lir(lir->lhs);
    char *rhs = print_lir(lir->rhs);
    const char *op = binary_name(lir->binary);

    return format("%s %s %s", op, lhs, rhs);
}

static char *print_unary(const lir_t *lir) {
    char *operand = print_lir(lir->operand);
    const char *op = unary_name(lir->unary);

    return format("%s %s", op, operand);
}

static char *print_module(const lir_t *lir) {
    vector_t *vars = VECTOR_MAP(lir->vars, print_lir);
    vector_t *funcs = VECTOR_MAP(lir->funcs, print_lir);

    return format("%s %s", strjoin(" ", vars), strjoin(" ", funcs));
}

static char *print_value(const lir_t *lir) {
    char *body = print_lir(lir->init);

    return format("%s %s", lir->name, body);
}

static char *print_define(const lir_t *lir) {
    char *body = print_lir(lir->body);

    return format("%s %s", lir->name, body);
}

static char *print_stmts(const lir_t *lir) {
    vector_t *stmts = VECTOR_MAP(lir->stmts, print_lir);

    return strjoin(" ", stmts);
}

static char *print_assign(const lir_t *lir) {
    char *dst = print_lir(lir->dst);
    char *src = print_lir(lir->src);

    return format("%s = %s", dst, src);
}

static char *print_while(const lir_t *lir) {
    char *cond = print_lir(lir->cond);
    char *body = print_lir(lir->then);

    return format("%s do %s", cond, body);
}

static char *print_branch(const lir_t *lir) {
    char *cond = print_lir(lir->cond);
    char *body = print_lir(lir->then);

    return format("%s then %s", cond, body);
}

char *print_lir(const lir_t *lir) {
    leaf_t leaf = lir->leaf;
    const char *name = leaf_name(leaf);
    char *body = NULL;

    depth += 1;

    switch (leaf) {
    case LIR_DIGIT: body = print_digit(lir); break;
    case LIR_NAME: body = print_name(lir); break;
    case LIR_BINARY: body = print_binary(lir); break;
    case LIR_UNARY: body = print_unary(lir); break;

    case LIR_WHILE: body = print_while(lir); break;
    case LIR_BRANCH: body = print_branch(lir); break;
    case LIR_STMTS: body = print_stmts(lir); break;
    case LIR_VALUE: body = print_value(lir); break;
    case LIR_DEFINE: body = print_define(lir); break;

    case LIR_ASSIGN: body = print_assign(lir); break;

    case LIR_MODULE: body = print_module(lir); break;

    default:
        break;
    }

    char *pad = strmul(" ", depth);

    depth -= 1;

    return format("\n%s(%s %s)", pad, name, body);
}
