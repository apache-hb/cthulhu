#pragma once

#include "cthulhu/tree/ops.h"
#include "scan/node.h"

#include <gmp.h>

typedef struct cpp_ast_t cpp_ast_t;
typedef struct scan_t scan_t;
typedef struct vector_t vector_t;

typedef enum cpp_kind_t
{
    // a define directive, may have an expression
    eCppDefine,

    // a macro definition, may have parameters and a body
    eCppMacro,

    // a macro parameter, can appear both in the macro definition and in the macro body
    // in the definition its treated as a parameter
    // in the variable its treated as a thing to be replaced when the macro is expanded
    eCppParam,

    // is a token defined
    eCppDefined,

    // a list of comma seperated tokens
    eCppList,

    eCppString,
    eCppIdent,
    eCppComma,
    eCppLParen,
    eCppRParen,
    eCppStringify,
    eCppConcat,
    eCppPaste,
    eCppNumber,

    eCppUnary,
    eCppBinary,
    eCppCompare,
    eCppTernary,
    eCppColon,

    eCppCount
} cpp_kind_t;

typedef struct cpp_params_t
{
    vector_t *names;
    bool variadic;
} cpp_params_t;

typedef struct cpp_number_t
{
    char *text;
    int base;
} cpp_number_t;

typedef struct cpp_ast_t
{
    cpp_kind_t kind;
    const node_t *node;

    union
    {
        // eCppDefine|eCppMacro
        struct
        {
            const char *name;
            cpp_params_t params;
            vector_t *body;
        };

        // eCppString|eCppIdent|eCppStringify|eCppPaste
        const char *text;

        struct
        {
            const char *original;
            mpz_t digit;
        };

        binary_t binary;
        unary_t unary;
        compare_t compare;
    };
} cpp_ast_t;

cpp_ast_t *cpp_define(scan_t *scan, where_t where, const char *name, vector_t *body);
cpp_ast_t *cpp_macro(scan_t *scan, where_t where, const char *name, cpp_params_t params, vector_t *body);

cpp_ast_t *cpp_string(scan_t *scan, where_t where, const char *text);
cpp_ast_t *cpp_ident(scan_t *scan, where_t where, const char *text);
cpp_ast_t *cpp_lparen(scan_t *scan, where_t where);
cpp_ast_t *cpp_rparen(scan_t *scan, where_t where);
cpp_ast_t *cpp_comma(scan_t *scan, where_t where);
cpp_ast_t *cpp_stringify(scan_t *scan, where_t where, const char *text);
cpp_ast_t *cpp_concat(scan_t *scan, where_t where);
cpp_ast_t *cpp_paste(scan_t *scan, where_t where, const char *text);
cpp_ast_t *cpp_number(scan_t *scan, where_t where, cpp_number_t number);
cpp_ast_t *cpp_number_int(arena_t *arena, const node_t *node, int i);

cpp_ast_t *cpp_binary(scan_t *scan, where_t where, binary_t op);
cpp_ast_t *cpp_unary(scan_t *scan, where_t where, unary_t op);
cpp_ast_t *cpp_compare(scan_t *scan, where_t where, compare_t op);
cpp_ast_t *cpp_ternary(scan_t *scan, where_t where);
cpp_ast_t *cpp_colon(scan_t *scan, where_t where);
cpp_ast_t *cpp_defined(scan_t *scan, where_t where);

bool cpp_ast_is(cpp_ast_t *ast, cpp_kind_t kind);
bool cpp_ast_is_not(cpp_ast_t *ast, cpp_kind_t kind);
