#pragma once

#include <gmp.h>

#include "cthulhu/ast/ops.h"
#include "cthulhu/ast/ast.h"

typedef enum {
    AST_PROGRAM,
    AST_MODULE,

    AST_VAR,
    AST_DEF,

    AST_NAME,
    AST_DIGIT,
    AST_CAST
} astof_t;

typedef enum {
    ATT_NAME,
    ATT_PTR
} attof_t;

typedef struct att_t {
    attof_t of;
    node_t *node;

    union {
        vector_t *path;
        struct att_t *ptr;
    };
} att_t;

typedef struct ast_t {
    astof_t of;
    node_t *node;

    union {
        mpz_t digit;
        
        vector_t *path;

        struct {
            struct ast_t *operand;
            struct att_t *cast;
        };

        struct {
            struct ast_t *modspec;
            vector_t *decls;
        };

        struct {
            char *name;

            struct att_t *type;
            struct ast_t *init;
        };
    };
} ast_t;

ast_t *ast_module(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_program(scan_t *scan, where_t where, ast_t *modspec, vector_t *decls);

ast_t *ast_vardecl(scan_t *scan, where_t where, char *name, att_t *type, ast_t *init);

ast_t *ast_name(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_digit(scan_t *scan, where_t where, mpz_t value);
ast_t *ast_cast(scan_t *scan, where_t where, ast_t *operand, att_t *cast);

att_t *att_name(scan_t *scan, where_t where, vector_t *path);
att_t *att_ptr(scan_t *scan, where_t where, att_t *ptr);
