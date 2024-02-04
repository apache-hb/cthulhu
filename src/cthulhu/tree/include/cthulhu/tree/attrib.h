#pragma once

#include "core/compiler.h"
#include "core/macros.h"

#include "cthulhu/tree/ops.h"

#include <stdint.h>

typedef struct vector_t vector_t;

CT_BEGIN_API

typedef struct tree_attrib_schema_t tree_attrib_schema_t;
typedef struct tree_attrib_t tree_attrib_t;

// a bitset of all the tree nodes that an attribute may be applied to
// this is used to validate that an attribute is only applied to the correct nodes
// TODO: maybe a generic bitset in the std module
typedef struct tree_apply_t
{
    uint_least64_t indices;
} tree_apply_t;

#define CT_TREE_APPLY(kind) (1ull << (kind))

CT_STATIC_ASSERT(eTreeTotal <= 64, "tree nodes have exceeded 64, tree apply needs to be updated");

/// @brief an attribute declaration from a target
/// this is in essence a function that can be applied to a tree node
typedef struct tree_attrib_schema_t
{
    /// @brief the name of the attribute
    const char *name;

    /// @brief which tree nodes this attribute can be applied to
    tree_apply_t apply;

    /// @brief the parameters this attribute takes
    /// vector_t<tree_t*> where each tree_t is a eTreeDeclParam
    const vector_t *params;
} tree_attrib_schema_t;

/// @brief an application of an attribute
typedef struct tree_attrib_t
{
    /// @brief the schema this attribute is applied to
    const tree_attrib_schema_t *schema;

    /// @brief the arguments this attribute was applied with
    /// vector_t<tree_t*> where each tree_t is an expression
    const vector_t *args;

    /// @brief the next attribute in the list
    tree_attrib_t *next;
} tree_attrib_t;

CT_TREE_API tree_attrib_t *tree_attrib_add(tree_attrib_t *root, IN_NOTNULL const tree_attrib_schema_t *schema, IN_NOTNULL const vector_t *args);

CT_END_API
