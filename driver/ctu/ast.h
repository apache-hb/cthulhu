#pragma once

#include <gmp.h>

#include "cthulhu/ast/ops.h"
#include "cthulhu/ast/ast.h"

typedef enum {
    AST_DIGIT,
    AST_BOOL,
} astof_t;

typedef struct ast_t {
    astof_t type;
    node_t *node;

    union {
        mpz_t digit;
        bool boolean;

        vector_t *parts;

        struct {
            struct ast_t *operand;
            unary_t unary;
        };

        struct {
            struct ast_t *lhs;
            struct ast_t *rhs;

            union {
                binary_t binary;
                compare_t compare;
            };
        };
    };
} ast_t;

ast_t *ast_name(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_digit(scan_t *scan, where_t where, mpz_t digit);
ast_t *ast_boolean(scan_t *scan, where_t where, bool boolean);
ast_t *ast_unary(scan_t *scan, where_t where, ast_t *operand, unary_t unary);
ast_t *ast_binary(scan_t *scan, where_t where, ast_t *lhs, ast_t *rhs, binary_t binary);
ast_t *ast_compare(scan_t *scan, where_t where, ast_t *lhs, ast_t *rhs, compare_t compare);
ast_t *ast_cast(scan_t *scan, where_t where, ast_t *operand, ast_t *type);

ast_t *ast_type(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_pointer(scan_t *scan, where_t where, ast_t *operand);
ast_t *ast_reference(scan_t *scan, where_t where, ast_t *operand);
