#pragma once

#include "scan.h"
#include <gmp.h>

#include "cthulhu/ast/ops.h"
#include "cthulhu/data/type.h"

typedef enum {
    AST_DIGIT,
    AST_IDENT,
    AST_STRING,
    AST_UNARY,
    AST_BINARY,
    AST_TERNARY
} astof_t;

typedef struct ast_t {
    astof_t type;
    node_t *node;

    union {
        mpz_t digit;

        const char *ident;

        struct {
            struct ast_t *operand;
            unary_t unary;
        };

        struct {
            struct ast_t *lhs;
            struct ast_t *rhs;
            binary_t binary;
        };
    };
} ast_t;

ast_t *ast_digit(scan_t *scan, where_t where, mpz_t digit);
ast_t *ast_ident(scan_t *scan, where_t where, const char *ident);
ast_t *ast_string(scan_t *scan, where_t where, const char *string);

ast_t *ast_unary(scan_t *scan, where_t where, ast_t *operand, unary_t unary);
ast_t *ast_binary(scan_t *scan, where_t where, ast_t *lhs, ast_t *rhs, binary_t binary);
ast_t *ast_ternary(scan_t *scan, where_t where, ast_t *cond, ast_t *then, ast_t *other);

void init_types(void);
type_t *get_digit(sign_t sign, digit_t digit);
type_t *get_void(void);
type_t *get_bool(void);
