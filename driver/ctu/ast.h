#pragma once

#include <gmp.h>

#include "cthulhu/ast/ast.h"
#include "cthulhu/ast/ops.h"

typedef enum
{
    AST_PROGRAM,
    AST_IMPORT,
    AST_MODULE,

    /* expressions */
    AST_DIGIT,
    AST_NAME,

    /* intermediate types */
    AST_TYPELIST,
    AST_FIELD,
    AST_CASE,

    /* types */
    AST_TYPENAME,
    AST_POINTER,
    AST_CLOSURE,
    AST_ARRAY,

    /* type declarations */
    AST_UNIONDECL,
    AST_STRUCTDECL,
    AST_ALIASDECL,
    AST_VARIANTDECL
} astof_t;

typedef struct ast_t
{
    astof_t of;
    const node_t *node;

    union {
        /* AST_TYPENAME|AST_MODULE */
        vector_t *path;

        /* AST_DIGIT */
        mpz_t digit;

        /* AST_POINTER|AST_ARRAY */
        struct
        {
            struct ast_t *type;

            union {
                bool indexable;
                struct ast_t *size;
            };
        };

        /* AST_CLOSURE|AST_TYPELIST */
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
            };
        };
    };
} ast_t;

/// module declarations

ast_t *ast_module(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_import(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_program(scan_t *scan, where_t where, ast_t *modspec, vector_t *imports, vector_t *decls);

/// expressions

ast_t *ast_digit(scan_t *scan, where_t where, mpz_t value);
ast_t *ast_name(scan_t *scan, where_t where, vector_t *path);

/// types

ast_t *ast_typename(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_pointer(scan_t *scan, where_t where, ast_t *type, bool indexable);
ast_t *ast_array(scan_t *scan, where_t where, ast_t *size, ast_t *type);
ast_t *ast_closure(scan_t *scan, where_t where, ast_t *args, ast_t *type);
ast_t *ast_typelist(vector_t *types, bool variadic);

/// type declarations

ast_t *ast_structdecl(scan_t *scan, where_t where, char *name, vector_t *fields);
ast_t *ast_uniondecl(scan_t *scan, where_t where, char *name, vector_t *fields);
ast_t *ast_typealias(scan_t *scan, where_t where, char *name, ast_t *type);
ast_t *ast_variantdecl(scan_t *scan, where_t where, char *name, vector_t *fields);

/// extra type data

ast_t *ast_field(scan_t *scan, where_t where, char *name, ast_t *type);
