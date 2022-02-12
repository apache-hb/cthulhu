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

static const char *NAMES[DIGIT_TOTAL][SIGN_TOTAL] = {
    [DIGIT_CHAR] = { 
        [SIGN_DEFAULT] = "char",
        [SIGN_UNSIGNED] = "unsigned char",
        [SIGN_SIGNED] = "signed char"
    },
    [DIGIT_SHORT] = { 
        [SIGN_DEFAULT] = "short",
        [SIGN_UNSIGNED] = "unsigned short",
        [SIGN_SIGNED] = "signed short"
    },
    [DIGIT_INT] = { 
        [SIGN_DEFAULT] = "int",
        [SIGN_UNSIGNED] = "unsigned int",
        [SIGN_SIGNED] = "signed int"
    },
    [DIGIT_LONG] = { 
        [SIGN_DEFAULT] = "long",
        [SIGN_UNSIGNED] = "unsigned long",
        [SIGN_SIGNED] = "signed long"
    }
};

static type_t *DIGITS[SIGN_TOTAL][DIGIT_TOTAL];
static type_t *VOID_TYPE;
static type_t *BOOL_TYPE;

void init_types(void) {
    for (int sign = 0; sign < SIGN_TOTAL; sign++) {
        for (int digit = 0; digit < DIGIT_TOTAL; digit++) {
            DIGITS[sign][digit] = type_digit(NAMES[digit][sign], NULL, sign, digit);
        }
    }

    VOID_TYPE = type_void("void");
    BOOL_TYPE = type_boolean("_Bool");
}

type_t *get_digit(sign_t sign, digit_t digit) {
    return DIGITS[sign][digit];
}

type_t *get_void(void) {
    return VOID_TYPE;
}

type_t *get_bool(void) {
    return BOOL_TYPE;
}
