#include "ast.h"

static ast_t *new_ast(astof_t type, scan_t *scan, where_t where) {
    ast_t *ast = ctu_malloc(sizeof(ast_t));
    ast->type = type;
    ast->node = node_new(scan, where);
    return ast;
}

ast_t *ast_digit(scan_t *scan, where_t where, mpz_t digit) {
    ast_t *ast = new_ast(AST_DIGIT, scan, where);
    mpz_init_set(ast->digit, digit);
    return ast;
}



static const char *SIGN_NAMES[SIGN_TOTAL] = {
    [SIGN_SIGNED] = "signed",
    [SIGN_UNSIGNED] = "unsigned",
    [SIGN_DEFAULT] = ""
};

static const char *DIGIT_NAMES[DIGIT_TOTAL] = {
    [DIGIT_CHAR] = "char",
    [DIGIT_SHORT] = "short",
    [DIGIT_INT] = "int",
    [DIGIT_LONG] = "long"
};

const char *get_name_for_sign(sign_t sign) {
    return SIGN_NAMES[sign];
}

const char *get_name_for_inttype(digit_t digit) {
    return DIGIT_NAMES[digit];
}

const char *get_name_for_digit(sign_t sign, digit_t digit) {
    return format("%s %s", get_name_for_sign(sign), get_name_for_inttype(digit));
}
