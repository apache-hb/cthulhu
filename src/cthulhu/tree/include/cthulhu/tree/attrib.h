#pragma once

#include "core/compiler.h"
#include "core/macros.h"

#include "cthulhu/tree/ops.h"

#include <stdint.h>

typedef struct vector_t vector_t;

CT_BEGIN_API

typedef struct tree_apply_t
{
    uint_least64_t indices;
} tree_apply_t;

#define CT_TREE_APPLY(kind) (1ull << (kind))

CT_STATIC_ASSERT(eTreeTotal <= 64, "tree nodes have exceeded 64, tree apply needs to be updated");

typedef struct tree_attrib_schema_t
{
    /// @brief the name of the attribute
    const char *name;

    /// @brief which tree nodes this attribute can be applied to
    tree_apply_t apply;

    /// @brief the parameters this attribute takes
    /// vector_t<tree_t*> where each tree_t is a eTreeDeclParam
    vector_t *params;
} tree_attrib_schema_t;

typedef struct tree_attrib_t
{
    /// @brief the schema this attribute is applied to
    const tree_attrib_schema_t *schema;

    /// @brief the arguments this attribute was applied with
    /// vector_t<tree_t*> where each tree_t is an expression
    vector_t *args;
} tree_attrib_t;

CT_END_API
