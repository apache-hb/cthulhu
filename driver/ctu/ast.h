#pragma once

#include "cthulhu/hlir/ops.h"
#include "scan/node.h"
#include "std/vector.h"

#include <gmp.h>

#include <stdbool.h>

typedef enum
{
    eAstProgram,
    eAstImport,
    eAstModule,
    eAstAttribute,

    /* expressions */
    eAstDigit,
    eAstBool,
    eAstName,
    eAstString,

    eAstUnary,
    eAstBinary,
    eAstCompare,

    eAstCall,

    /* statements */
    eAstStmts,
    eAstReturn,
    eAstWhile,
    eAstBreak,
    eAstContinue,

    /* intermediate types */
    eAstField,
    eAstCase,
    eAstParam,

    /* types */
    eAstTypename,
    eAstPointer,
    eAstClosure,
    eAstArray,

    /* type declarations */
    eAstDeclUnion,
    eAstDeclStruct,
    eAstDeclAlias,
    eAstDeclVariant,

    /* declarations */
    eAstFunction,
    eAstVariable
} astof_t;

typedef struct ast_t
{
    astof_t of;
    node_t node;

    union {
        /* eAstTypename|eAstModule */
        vector_t *path;

        /* eAstDigit */
        mpz_t digit;

        /* eAstBool */
        bool boolean;

        /* eAstString */
        struct
        {
            char *string;
            size_t length;
        };

        /* eAstBinary */
        struct
        {
            union {
                binary_t binary;
                compare_t compare;
            };

            struct ast_t *lhs;
            struct ast_t *rhs;
        };

        /* eAstUnary|eAstReturn */
        struct
        {
            unary_t unary;
            struct ast_t *operand;
        };

        /* eAstPointer|eAstArray */
        struct
        {
            struct ast_t *type;

            union {
                bool indexable;
                struct ast_t *size;
            };
        };

        /* eAstClosure */
        struct
        {
            vector_t *params;
            struct ast_t *result;
            bool variadic;
        };

        /* eAstWhile */
        struct
        {
            struct ast_t *cond;
            struct ast_t *then;
            struct ast_t *other;
        };

        /* eAstProgram */
        struct
        {
            struct ast_t *modspec;
            vector_t *imports;
            vector_t *decls;
        };

        struct
        {
            struct ast_t *call;
            vector_t *args;
        };

        /* eAstStmts */
        vector_t *stmts;

        struct
        {
            char *name;
            struct ast_t *attrib;

            union {
                /* eAstField */
                struct ast_t *field;

                /* eAstDeclUnion|eAstDeclStruct|eAstDeclVariant */
                vector_t *fields;

                /* eAstDeclAlias */
                struct {
                    bool newtype;
                    struct ast_t *alias;
                };

                /* eAstParam */
                struct ast_t *param;

                /* eAstAttribute */
                vector_t *config;

                /* eAstVariable */
                struct
                {
                    bool mut;
                    struct ast_t *expected;
                    struct ast_t *init;
                };

                /* eAstFunction */
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

ast_t *ast_module(scan_t scan, where_t where, vector_t *path);
ast_t *ast_import(scan_t scan, where_t where, vector_t *path);
ast_t *ast_program(scan_t scan, where_t where, ast_t *modspec, vector_t *imports, vector_t *decls);

ast_t *ast_attribute(scan_t scan, where_t where, char *name, vector_t *args);

/// declarations

ast_t *ast_function(scan_t scan, where_t where, char *name, ast_t *signature, ast_t *body);
ast_t *ast_variable(scan_t scan, where_t where, char *name, bool mut, ast_t *expected, ast_t *init);

/// expressions

ast_t *ast_digit(scan_t scan, where_t where, mpz_t value);
ast_t *ast_bool(scan_t scan, where_t where, bool value);
ast_t *ast_name(scan_t scan, where_t where, vector_t *path);
ast_t *ast_string(scan_t scan, where_t where, char *str, size_t len);

ast_t *ast_unary(scan_t scan, where_t where, unary_t op, ast_t *operand);
ast_t *ast_binary(scan_t scan, where_t where, binary_t binary, ast_t *lhs, ast_t *rhs);
ast_t *ast_compare(scan_t scan, where_t where, compare_t compare, ast_t *lhs, ast_t *rhs);

ast_t *ast_call(scan_t scan, where_t where, ast_t *call, vector_t *args);

/// statements

ast_t *ast_stmts(scan_t scan, where_t where, vector_t *stmts);
ast_t *ast_return(scan_t scan, where_t where, ast_t *expr);
ast_t *ast_while(scan_t scan, where_t where, ast_t *cond, ast_t *body, ast_t *other);

ast_t *ast_break(scan_t scan, where_t where);
ast_t *ast_continue(scan_t scan, where_t where);

/// types

ast_t *ast_typename(scan_t scan, where_t where, vector_t *path);
ast_t *ast_pointer(scan_t scan, where_t where, ast_t *type, bool indexable);
ast_t *ast_array(scan_t scan, where_t where, ast_t *size, ast_t *type);
ast_t *ast_closure(scan_t scan, where_t where, vector_t *params, bool variadic, ast_t *type);

/// type declarations

ast_t *ast_structdecl(scan_t scan, where_t where, char *name, vector_t *fields);
ast_t *ast_uniondecl(scan_t scan, where_t where, char *name, vector_t *fields);
ast_t *ast_typealias(scan_t scan, where_t where, char *name, bool newtype, ast_t *type);
ast_t *ast_variantdecl(scan_t scan, where_t where, char *name, vector_t *fields);

/// extra type data

ast_t *ast_field(scan_t scan, where_t where, char *name, ast_t *type);
ast_t *ast_param(scan_t scan, where_t where, char *name, ast_t *type);

/// inner ast types

typedef struct
{
    vector_t *params;
    bool variadic;
} funcparams_t;

funcparams_t funcparams_new(vector_t *params, bool variadic);
