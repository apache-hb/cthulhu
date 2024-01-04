#pragma once

#include <gmp.h>

#include "cthulhu/tree/ops.h" // IWYU pragma: export
#include "scan/node.h"

typedef struct vector_t vector_t;

// TODO: parse C

typedef enum c_kind_t
{
    eAstTypedef,
    eAstStruct,
    eAstUnion,
    eAstEnum,

    eAstEnumValue,
    eAstTypeField,

    // synthetic intermediate nodes
    eAstOpaque,

    eAstCount
} c_kind_t;

typedef struct c_ast_t
{
    const node_t *node;
    c_kind_t kind;

    union {
        struct {
            const char *name;

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

c_ast_t *c_ast_typedef(const scan_t *scan, where_t where, const char *name, c_ast_t *alias);
c_ast_t *c_ast_struct(const scan_t *scan, where_t where, const char *name, vector_t *fields);
c_ast_t *c_ast_union(const scan_t *scan, where_t where, const char *name, vector_t *fields);
c_ast_t *c_ast_enum(const scan_t *scan, where_t where, const char *name, vector_t *fields);
c_ast_t *c_ast_enum_value(const scan_t *scan, where_t where, const char *name, c_ast_t *value);
c_ast_t *c_ast_type_field(const scan_t *scan, where_t where, const char *name, c_ast_t *type);

// forward declarations
c_ast_t *c_ast_opaque(const scan_t *scan, where_t where, c_kind_t expected);
