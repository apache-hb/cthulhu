#pragma once

#include "cthulhu/hlir/ops.h"
#include "scan/node.h"
#include "std/vector.h"

#include "scan.h"

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
    eAstDecimal,
    eAstBool,
    eAstName,
    eAstString,
    eAstNull,
    eAstDefault, // default value

    eAstUnary,
    eAstBinary,
    eAstCompare,

    eAstAccess,
    eAstCall,
    eAstIndex,
    eAstInit,

    eAstRef,
    eAstDeref,
    eAstSizeOf,

    /* statements */
    eAstStmts,
    eAstReturn,
    eAstWhile,
    eAstBranch,
    eAstAssign,
    eAstBreak,
    eAstContinue,

    /* intermediate types */
    eAstField,
    eAstCase,
    eAstParam,
    eAstFieldInit,

    /* types */
    eAstTypename,
    eAstPointer,
    eAstClosure,
    eAstArray,
    eAstVarArgs,

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
    node_t *node;

    union {
        /* eAstTypename|eAstModule|eAstImport */
        struct 
        {
            vector_t *path; // import path
            vector_t *items; // import items
            char *id; // import name
        };

        /* eAstDigit */
        struct
        {
            mpz_t digit;
            char *suffix;
        };

        /* eAstDecimal */
        mpq_t decimal;

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

                /* eAstInit */
                vector_t *inits;
            };
        };

        /* eAstClosure */
        struct
        {
            vector_t *params;
            struct ast_t *result;
        };

        /* eAstWhile | eAstBranch */
        struct
        {
            char *label;
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

        /* eAstAccess */
        struct
        {
            struct ast_t *record;
            const char *access;
            bool indirect;
        };

        /* eAstIndex */
        struct
        {
            struct ast_t *array;
            struct ast_t *index;
        };

        struct
        {
            struct ast_t *dst;
            struct ast_t *src;
        };

        /* eAstStmts */
        vector_t *stmts;

        struct
        {
            char *name;
            vector_t *attribs;
            bool exported;

            union {
                /* eAstField */
                struct {
                    struct ast_t *field;

                    /* eAstFieldInit */
                    struct ast_t *value;
                };

                /* eAstDeclUnion|eAstDeclStruct|eAstDeclVariant */
                struct {
                    struct ast_t *underlying;
                    vector_t *fields;
                };

                /* eAstDeclAlias */
                struct 
                {
                    struct ast_t *alias;
                    bool newtype;
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

                struct
                {
                    vector_t *caseFields;
                    struct ast_t *caseValue;
                    bool isCaseDefault;
                };
            };
        };
    };
} ast_t;

/// module declarations

ast_t *ast_module(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_import(scan_t *scan, where_t where, vector_t *path, char *alias, vector_t *items);
ast_t *ast_program(scan_t *scan, where_t where, ast_t *modspec, vector_t *imports, vector_t *decls);

ast_t *ast_attribute(scan_t *scan, where_t where, char *name, vector_t *args);

/// declarations

ast_t *ast_function(scan_t *scan, where_t where, char *name, ast_t *signature, ast_t *body);
ast_t *ast_variable(scan_t *scan, where_t where, char *name, bool mut, ast_t *expected, ast_t *init);

/// expressions

ast_t *ast_digit(scan_t *scan, where_t where, mpz_t value, char *suffix);
ast_t *ast_decimal(scan_t *scan, where_t where, mpq_t value);
ast_t *ast_bool(scan_t *scan, where_t where, bool value);
ast_t *ast_name(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_string(scan_t *scan, where_t where, char *str, size_t len);
ast_t *ast_null(scan_t *scan, where_t where);

ast_t *ast_unary(scan_t *scan, where_t where, unary_t op, ast_t *operand);
ast_t *ast_binary(scan_t *scan, where_t where, binary_t binary, ast_t *lhs, ast_t *rhs);
ast_t *ast_compare(scan_t *scan, where_t where, compare_t compare, ast_t *lhs, ast_t *rhs);

ast_t *ast_access(scan_t *scan, where_t where, ast_t *data, const char *field, bool indirect);
ast_t *ast_index(scan_t *scan, where_t where, ast_t *array, ast_t *index);

ast_t *ast_call(scan_t *scan, where_t where, ast_t *call, vector_t *args);

ast_t *ast_ref(scan_t *scan, where_t where, ast_t *value);
ast_t *ast_deref(scan_t *scan, where_t where, ast_t *value);
ast_t *ast_sizeof(scan_t *scan, where_t where, ast_t *type);

ast_t *ast_init(scan_t *scan, where_t where, ast_t *type, vector_t *fields);
ast_t *ast_default(scan_t *scan, where_t where);

/// statements

ast_t *ast_stmts(scan_t *scan, where_t where, vector_t *stmts);
ast_t *ast_return(scan_t *scan, where_t where, ast_t *expr);
ast_t *ast_while(scan_t *scan, where_t where, char *label, ast_t *cond, ast_t *body, ast_t *other);

ast_t *ast_assign(scan_t *scan, where_t where, ast_t *dst, ast_t *src);

ast_t *ast_branch(scan_t *scan, where_t where, ast_t *cond, ast_t *body, ast_t *other);

ast_t *ast_break(scan_t *scan, where_t where, char *label);
ast_t *ast_continue(scan_t *scan, where_t where, char *label);

/// types

ast_t *ast_typename(scan_t *scan, where_t where, vector_t *path);
ast_t *ast_pointer(scan_t *scan, where_t where, ast_t *type, bool indexable);
ast_t *ast_array(scan_t *scan, where_t where, ast_t *size, ast_t *type);
ast_t *ast_closure(scan_t *scan, where_t where, vector_t *params, ast_t *type);
ast_t *ast_varargs(scan_t *scan, where_t where);

/// type declarations

ast_t *ast_structdecl(scan_t *scan, where_t where, char *name, vector_t *fields);
ast_t *ast_uniondecl(scan_t *scan, where_t where, char *name, vector_t *fields);
ast_t *ast_typealias(scan_t *scan, where_t where, char *name, ast_t *type);
ast_t *ast_variantdecl(scan_t *scan, where_t where, char *name, ast_t *type, vector_t *fields);
ast_t *ast_newtype(scan_t *scan, where_t where, char *name, ast_t *type);

/// extra type data

ast_t *ast_field(scan_t *scan, where_t where, char *name, ast_t *type, ast_t *value);
ast_t *ast_param(scan_t *scan, where_t where, char *name, ast_t *type);
ast_t *ast_fieldinit(scan_t *scan, where_t where, char *name, ast_t *value);

ast_t *ast_case(scan_t *scan, where_t where, char *name, vector_t *fields, ast_t *value, bool isDefault);

void set_attribs(ast_t *decl, bool exported, vector_t *attribs);
