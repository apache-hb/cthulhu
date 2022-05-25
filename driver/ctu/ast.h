#pragma once

#include "cthulhu/ast/ast.h"
#include "cthulhu/hlir/ops.h"
#include "cthulhu/util/vector.h"

#include <gmp.h>

#include <stdbool.h>

typedef enum
{
    AST_PROGRAM,
    AST_IMPORT,
    AST_MODULE,

    /* expressions */
    AST_DIGIT,
    AST_BOOL,
    AST_NAME,

    AST_UNARY,
    AST_BINARY,

    /* statements */
    AST_STMTS,
    AST_RETURN,

    /* intermediate types */
    AST_FIELD,
    AST_CASE,
    AST_PARAM,

    /* types */
    AST_TYPENAME,
    AST_POINTER,
    AST_CLOSURE,
    AST_ARRAY,

    /* type declarations */
    AST_UNIONDECL,
    AST_STRUCTDECL,
    AST_ALIASDECL,
    AST_VARIANTDECL,

    /* declarations */
    AST_FUNCDECL
} astof_t;

typedef struct ast_t
{
    astof_t of;
    node_t node;

    union {
        /* AST_TYPENAME|AST_MODULE */
        vector_t *path;

        /* AST_DIGIT */
        mpz_t digit;

        /* AST_BOOL */
        bool boolean;

        /* AST_BINARY */
        struct
        {
            binary_t binary;
            struct ast_t *lhs;
            struct ast_t *rhs;
        };

        /* AST_UNARY|AST_RETURN */
        struct
        {
            unary_t unary;
            struct ast_t *operand;
        };

        /* AST_POINTER|AST_ARRAY */
        struct
        {
            struct ast_t *type;

            union {
                bool indexable;
                struct ast_t *size;
            };
        };

        /* AST_CLOSURE */
        struct
        {
            vector_t *params;
            struct ast_t *result;
            bool variadic;
        };

        /* AST_PROGRAM */
        struct
        {
            struct ast_t *modspec;
            vector_t *imports;
            vector_t *decls;
        };

        /* AST_STMTS */
        vector_t *stmts;

        struct
        {
            char *name;

            union {
                /* AST_FIELD */
                struct ast_t *field;

                /* AST_UNIONDECL|AST_STRUCTDECL|AST_VARIANTDECL */
                vector_t *fields;

                /* AST_ALIASDECL */
                struct ast_t *alias;

                /* AST_PARAM */
                struct ast_t *param;

                /* AST_FUNCDECL */
                struct
                {
                    struct ast_t *signature;
                    struct ast_t *body;
                };
            };
        };
    };
} ast_t;

/// module declarations

ast_t *ast_module(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_import(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_program(scan_t *scan, where_t where, ast_t *modspec, vector_t *imports, vector_t *decls);

/// declarations

ast_t *ast_funcdecl(scan_t *scan, where_t where, char *name, ast_t *signature, ast_t *body);

/// expressions

ast_t *ast_digit(scan_t *scan, where_t where, mpz_t value);
ast_t *ast_bool(scan_t *scan, where_t where, bool value);
ast_t *ast_name(scan_t *scan, where_t where, vector_t *path);

ast_t *ast_unary(scan_t *scan, where_t where, unary_t op, ast_t *operand);
ast_t *ast_binary(scan_t *scan, where_t where, binary_t binary, ast_t *lhs, ast_t *rhs);

/// statements

ast_t *ast_stmts(scan_t *scan, where_t where, vector_t *stmts);
ast_t *ast_return(scan_t *scan, where_t where, ast_t *expr);

/// types

ast_t *ast_typename(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_pointer(scan_t *scan, where_t where, ast_t *type, bool indexable);
ast_t *ast_array(scan_t *scan, where_t where, ast_t *size, ast_t *type);
ast_t *ast_closure(scan_t *scan, where_t where, vector_t *params, bool variadic, ast_t *type);

/// type declarations

ast_t *ast_structdecl(scan_t *scan, where_t where, char *name, vector_t *fields);
ast_t *ast_uniondecl(scan_t *scan, where_t where, char *name, vector_t *fields);
ast_t *ast_typealias(scan_t *scan, where_t where, char *name, ast_t *type);
ast_t *ast_variantdecl(scan_t *scan, where_t where, char *name, vector_t *fields);

/// extra type data

ast_t *ast_field(scan_t *scan, where_t where, char *name, ast_t *type);
ast_t *ast_param(scan_t *scan, where_t where, char *name, ast_t *type);

/// inner ast types

typedef struct
{
    vector_t *params;
    bool variadic;
} funcparams_t;

funcparams_t funcparams_new(vector_t *params, bool variadic);
