// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <gmp.h>

#include "cthulhu/tree/ops.h" // IWYU pragma: export
#include "scan/node.h"

typedef struct vector_t vector_t;
typedef struct typevec_t typevec_t;

typedef enum c_kind_t
{
#define C_AST_KIND(id, name) id,
#include "c/ast.inc"

    eAstCount
} c_kind_t;

typedef enum c_storage_class_t
{
#define C_STORAGE_CLASS(id, name, v) id = (v),
#include "c/ast.inc"
} c_storage_class_t;

typedef enum c_type_qualifier_t
{
#define C_TYPE_QUALIFIER(id, name, v) id = (v),
#include "c/ast.inc"
} c_type_qualifier_t;

typedef enum c_type_specifier_t
{
#define C_TYPE_SPECIFIER(id, name) id,
#include "c/ast.inc"
    eTypeSpecifierCount
} c_type_specifier_t;

typedef enum c_callconv_t
{
#define C_CALLCONV(id, name) id,
#include "c/ast.inc"
} c_callconv_t;

typedef struct c_ast_t
{
    const node_t *node;
    c_kind_t kind;

    union {
        // eAstLabel, eAstGoto
        const char *label;

        // eAstCase
        struct c_ast_t *case_value;

        // eAstAttributeCallConv
        c_callconv_t callconv;

        // eAstStorageClass
        c_storage_class_t storage_class;

        // eAstTypeQualifier
        c_type_qualifier_t type_qualifier;

        // eAstTypeSpecifier
        c_type_specifier_t type_specifier;

        // eAstTypedefName
        char *typedef_name;

        // eAstTypeSpecifier
        vector_t *typename_parts;

        // eAstAlignas
        struct c_ast_t *alignas_body;

        // eAstAlignof
        struct c_ast_t *alignof_body;

        // eAstSizeof
        struct c_ast_t *sizeof_body;

        // eAstExprList
        vector_t *exprs;

        struct {
            struct c_ast_t *cast_type;
            struct c_ast_t *operand;
        };

        // eAstDeclaratorList
        struct {
            vector_t *specifiers;
            vector_t *declarations;
        };

        // eAstInitDeclarator
        struct {
            struct c_ast_t *declarator;
            struct c_ast_t *initializer;
        };

        // eAstString
        typevec_t *string;

        // eAstReturn
        struct c_ast_t *result;

        struct {
            bool exported;
            vector_t *attributes;

            union {
                // eAstModulePublicFragment, eAstModuleImport
                vector_t *module_path;

                // anything else with a name
                const char *name;
            };

            union {
                // eAstTypedef
                struct c_ast_t *alias;

                // eAstTypeField
                struct c_ast_t *type;

                // eAstStruct, eAstUnion, eAstEnum
                vector_t *fields;

                // eAstEnumValue
                struct c_ast_t *value;

                // eAstOpaque
                c_kind_t expected;
            };
        };
    };
} c_ast_t;

// module extensions
c_ast_t *c_ast_module_private_fragment(scan_t *scan, where_t where);
c_ast_t *c_ast_module_public_fragment(scan_t *scan, where_t where, vector_t *module_path);
c_ast_t *c_ast_module_import(scan_t *scan, where_t where, vector_t *module_path);

c_ast_t *c_ast_declarator_list(scan_t *scan, where_t where, vector_t *specifiers, vector_t *declarations);
c_ast_t *c_ast_init_declarator(scan_t *scan, where_t where, c_ast_t *declarator, c_ast_t *initializer);

c_ast_t *c_ast_storage_class(scan_t *scan, where_t where, c_storage_class_t storage_class);
c_ast_t *c_ast_type_qualifier(scan_t *scan, where_t where, c_type_qualifier_t qualifier);
c_ast_t *c_ast_type_specifier(scan_t *scan, where_t where, c_type_specifier_t specifier);
c_ast_t *c_ast_typedef_name(scan_t *scan, where_t where, char *name);
c_ast_t *c_ast_type(scan_t *scan, where_t where, vector_t *parts);

c_ast_t *c_ast_alignas(scan_t *scan, where_t where, c_ast_t *body);

// expressions
c_ast_t *c_ast_expr_compare(scan_t *scan, where_t where, c_ast_t *lhs, c_ast_t *rhs, compare_t compare);
c_ast_t *c_ast_expr_binary(scan_t *scan, where_t where, c_ast_t *lhs, c_ast_t *rhs, binary_t binary);
c_ast_t *c_ast_expr_unary(scan_t *scan, where_t where, c_ast_t *expr, unary_t unary);
c_ast_t *c_ast_expr_ternary(scan_t *scan, where_t where, c_ast_t *cond, c_ast_t *truthy, c_ast_t *falsey);
c_ast_t *c_ast_expr_assign(scan_t *scan, where_t where, c_ast_t *lhs, c_ast_t *rhs, binary_t op);
c_ast_t *c_ast_expr_list(scan_t *scan, where_t where, vector_t *exprs);
c_ast_t *c_ast_expand_exprs(scan_t *scan, where_t where, vector_t *exprs);
c_ast_t *c_ast_cast(scan_t *scan, where_t where, c_ast_t *expr, c_ast_t *type);

c_ast_t *c_ast_string(scan_t *scan, where_t where, text_t text);
c_ast_t *c_ast_append_string(scan_t *scan, where_t where, c_ast_t *string, text_t text);

// statements
c_ast_t *c_ast_goto(scan_t *scan, where_t where, char *label);
c_ast_t *c_ast_continue(scan_t *scan, where_t where);
c_ast_t *c_ast_break(scan_t *scan, where_t where);
c_ast_t *c_ast_return(scan_t *scan, where_t where, c_ast_t *value);

// attributes
c_ast_t *c_ast_attribute(scan_t *scan, where_t where, vector_t *body);
c_ast_t *c_ast_attribute_noreturn(scan_t *scan, where_t where);
c_ast_t *c_ast_attribute_callconv(scan_t *scan, where_t where, c_callconv_t callconv);

// actual declarations
c_ast_t *c_ast_typedef(scan_t *scan, where_t where, const char *name, c_ast_t *alias);
c_ast_t *c_ast_struct(scan_t *scan, where_t where, const char *name, vector_t *fields);
c_ast_t *c_ast_union(scan_t *scan, where_t where, const char *name, vector_t *fields);
c_ast_t *c_ast_enum(scan_t *scan, where_t where, const char *name, vector_t *fields);
c_ast_t *c_ast_enum_value(scan_t *scan, where_t where, const char *name, c_ast_t *value);
c_ast_t *c_ast_type_field(scan_t *scan, where_t where, const char *name, c_ast_t *type);

// forward declarations
c_ast_t *c_ast_opaque(scan_t *scan, where_t where, c_kind_t expected);

c_ast_t *c_ast_label(scan_t *scan, where_t where, const char *name);
c_ast_t *c_ast_case(scan_t *scan, where_t where, c_ast_t *value);
c_ast_t *c_ast_default(scan_t *scan, where_t where);

void c_ast_apply_export(c_ast_t *ast, bool exported);
void c_ast_apply_attributes(c_ast_t *ast, vector_t *attributes);
