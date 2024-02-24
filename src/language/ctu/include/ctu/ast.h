// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "scan/scan.h"
#include "scan/node.h"

#include "cthulhu/tree/ops.h"

#include <gmp.h>

typedef struct ctu_t ctu_t;
typedef struct vector_t vector_t;

typedef enum ctu_kind_t {
    /* expressions */
    eCtuExprInt,
    eCtuExprBool,
    eCtuExprString,
    eCtuExprName,
    eCtuExprCast,
    eCtuExprInit,

    eCtuExprCompare,
    eCtuExprBinary,
    eCtuExprUnary,
    eCtuExprCall,
    eCtuExprIndex,
    eCtuExprField,
    eCtuExprFieldIndirect,

    eCtuExprRef,
    eCtuExprDeref,

    /* statements */
    eCtuStmtList,
    eCtuStmtLocal,
    eCtuStmtReturn,
    eCtuStmtWhile,
    eCtuStmtAssign,
    eCtuStmtBreak,
    eCtuStmtContinue,
    eCtuStmtBranch,

    /* types */
    eCtuTypeName,
    eCtuTypePointer,
    eCtuTypeFunction,
    eCtuTypeArray,

    /* real decls */
    eCtuDeclGlobal,
    eCtuDeclFunction,

    /* type decls */
    eCtuDeclTypeAlias,
    eCtuDeclVariant,
    eCtuDeclUnion,
    eCtuDeclStruct,

    /* intermediates */
    eCtuField,
    eCtuParam,
    eCtuAttrib,
    eCtuFieldInit,
    eCtuVariantCase,

    /* modules */
    eCtuImport,
    eCtuModule
} ctu_kind_t;

typedef struct ctu_t {
    ctu_kind_t kind;
    const node_t *node;

    union {
        struct {
            char *name;
            bool exported;
            const vector_t *attribs;

            union {
                /* eCtuImport */
                const vector_t *import_path;

                /* eCtuStmtWhile */
                struct {
                    ctu_t *cond;
                    ctu_t *then;
                    ctu_t *other;
                };

                /* eCtuGlobal|eCtuStmtLocal */
                struct {
                    ctu_t *type;
                    ctu_t *value;
                    bool mut;
                };

                /* eCtuDeclFunction|eCtuTypeFunction */
                struct {
                    const vector_t *params;
                    char *variadic;
                    ctu_t *return_type;
                    ctu_t *body;
                };

                /* eCtuDeclTypeAlias */
                struct {
                    bool newtype;
                    ctu_t *type_alias;
                };

                /* eCtuDeclStruct|eCtuDeclUnion */
                const vector_t *fields;

                /* eCtuField */
                ctu_t *field_type;

                /* eCtuParam */
                ctu_t *param_type;

                /* eCtuDeclVariant */
                struct {
                    ctu_t *underlying;
                    vector_t *cases;
                };

                /* eCtuVariantCase */
                struct {
                    bool default_case;
                    ctu_t *case_value;
                };
            };
        };

        /* eCtuExprInt */
        mpz_t int_value;

        /* eCtuExprBool */
        bool bool_value;

        /* eCtuExprString */
        struct {
            char *text;
            size_t length;
        };

        /* eCtuExprInit */
        const vector_t *inits;

        /* eCtuExprName */
        const vector_t *path;

        struct {
            union {
                /* eCtuExprBinary */
                binary_t binary;

                /* eCtuExprCompare */
                compare_t compare;
            };

            ctu_t *lhs;
            ctu_t *rhs;
        };

        struct {
            union {
                /* eCtuExprIndex */
                ctu_t *index;

                /* eCtuExprUnary */
                unary_t unary;

                /* eCtuExprField|eCtuExprFieldIndirect|eCtuFieldInit */
                char *field;

                /* eCtuExprCast */
                ctu_t *cast;
            };

            ctu_t *expr;
        };

        /* eCtuExprCall */
        struct {
            ctu_t *callee;
            const vector_t *args;
        };

        /* eCtuStmtList */
        const vector_t *stmts;

        /* eCtuStmtReturn */
        ctu_t *result;

        /* eCtuStmtAssign */
        struct {
            ctu_t *dst;
            ctu_t *src;
        };

        /* eCtuStmtBreak|eCtuStmtContinue */
        char *label;

        /* eCtuTypeName */
        vector_t *type_name;

        /* eCtuTypePointer */
        ctu_t *pointer;

        /* eCtuTypeArray */
        struct {
            ctu_t *array_type;
            ctu_t *array_length;
        };

        /* eCtuAttrib */
        struct {
            const vector_t *attrib_path;
            const vector_t *attrib_args;
        };

        /* eCtuModule */
        struct {
            const vector_t *modspec;
            const vector_t *imports;
            const vector_t *decls;
        };
    };
} ctu_t;

///
/// modules
///

ctu_t *ctu_module(scan_t *scan, where_t where, const vector_t *modspec, const vector_t *imports, const vector_t *decls);
ctu_t *ctu_import(scan_t *scan, where_t where, vector_t *path, char *name);

///
/// decorators
///

ctu_t *ctu_attrib(scan_t *scan, where_t where, const vector_t *path, const vector_t *args);
ctu_t *ctu_apply(ctu_t *decl, const vector_t *attribs);

///
/// statements
///

ctu_t *ctu_stmt_list(scan_t *scan, where_t where, vector_t *stmts);
ctu_t *ctu_stmt_local(scan_t *scan, where_t where, bool mutable, char *name, ctu_t *type, ctu_t *value);
ctu_t *ctu_stmt_return(scan_t *scan, where_t where, ctu_t *value);
ctu_t *ctu_stmt_while(scan_t *scan, where_t where, char *name, ctu_t *cond, ctu_t *then, ctu_t *other);
ctu_t *ctu_stmt_assign(scan_t *scan, where_t where, ctu_t *dst, ctu_t *src);
ctu_t *ctu_stmt_break(scan_t *scan, where_t where, char *label);
ctu_t *ctu_stmt_continue(scan_t *scan, where_t where, char *label);

ctu_t *ctu_stmt_branch(scan_t *scan, where_t where, ctu_t *cond, ctu_t *then, ctu_t *other);

///
/// expressions
///

ctu_t *ctu_expr_int(scan_t *scan, where_t where, mpz_t value);
ctu_t *ctu_expr_bool(scan_t *scan, where_t where, bool value);
ctu_t *ctu_expr_string(scan_t *scan, where_t where, char *text, size_t length);
ctu_t *ctu_expr_init(scan_t *scan, where_t where, const vector_t *inits);

ctu_t *ctu_expr_call(scan_t *scan, where_t where, ctu_t *callee, const vector_t *args);
ctu_t *ctu_expr_name(scan_t *scan, where_t where, const vector_t *path);
ctu_t *ctu_expr_cast(scan_t *scan, where_t where, ctu_t *expr, ctu_t *type);

ctu_t *ctu_expr_ref(scan_t *scan, where_t where, ctu_t *expr);
ctu_t *ctu_expr_deref(scan_t *scan, where_t where, ctu_t *expr);
ctu_t *ctu_expr_index(scan_t *scan, where_t where, ctu_t *expr, ctu_t *index);
ctu_t *ctu_expr_field(scan_t *scan, where_t where, ctu_t *expr, char *field);
ctu_t *ctu_expr_field_indirect(scan_t *scan, where_t where, ctu_t *expr, char *field);

ctu_t *ctu_expr_unary(scan_t *scan, where_t where, unary_t unary, ctu_t *expr);
ctu_t *ctu_expr_binary(scan_t *scan, where_t where, binary_t binary, ctu_t *lhs, ctu_t *rhs);
ctu_t *ctu_expr_compare(scan_t *scan, where_t where, compare_t compare, ctu_t *lhs, ctu_t *rhs);

///
/// types
///

ctu_t *ctu_type_name(scan_t *scan, where_t where, vector_t *path);
ctu_t *ctu_type_pointer(scan_t *scan, where_t where, ctu_t *pointer);
ctu_t *ctu_type_array(scan_t *scan, where_t where, ctu_t *array, ctu_t *length);
ctu_t *ctu_type_function(scan_t *scan, where_t where, const vector_t *params, ctu_t *return_type);

///
/// real declarations
///

ctu_t *ctu_decl_global(scan_t *scan, where_t where, bool exported, bool mutable, char *name, ctu_t *type, ctu_t *value);
ctu_t *ctu_decl_function(scan_t *scan, where_t where, bool exported, char *name, const vector_t *params, char *variadic, ctu_t *return_type, ctu_t *body);

///
/// type declarations
///

ctu_t *ctu_decl_typealias(scan_t *scan, where_t where, bool exported, char *name, bool newtype, ctu_t *type);

ctu_t *ctu_decl_union(scan_t *scan, where_t where, bool exported, char *name, vector_t *fields);
ctu_t *ctu_decl_struct(scan_t *scan, where_t where, bool exported, char *name, vector_t *fields);

ctu_t *ctu_decl_variant(scan_t *scan, where_t where, bool exported, char *name, ctu_t *underlying, vector_t *cases);

///
/// internal components
///

ctu_t *ctu_field(scan_t *scan, where_t where, char *name, ctu_t *type);
ctu_t *ctu_param(scan_t *scan, where_t where, char *name, ctu_t *type);
ctu_t *ctu_field_init(scan_t *scan, where_t where, char *name, ctu_t *value);
ctu_t *ctu_variant_case(scan_t *scan, where_t where, char *name, bool is_default, ctu_t *expr);

///
/// extras
///

typedef struct ctu_params_t {
    const vector_t *params;
    char *variadic;
} ctu_params_t;

ctu_params_t ctu_params_new(const vector_t *params, char *variadic);
