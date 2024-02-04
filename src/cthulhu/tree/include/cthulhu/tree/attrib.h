#pragma once

#include <ctu_tree_api.h>

#include "core/analyze.h"
#include "core/compiler.h"

#include "std/bitset.h"

#include <stdint.h>

typedef struct vector_t vector_t;

CT_BEGIN_API

typedef struct tree_t tree_t;
typedef struct node_t node_t;
typedef struct tree_attrib_t tree_attrib_t;

/// @brief an application of an attribute
typedef struct tree_attrib_t
{
    /// @brief the schema this attribute is applied to
    const tree_t *schema;

    /// @brief the arguments this attribute was applied with
    /// vector_t<tree_t*> where each tree_t is an expression
    const vector_t *args;

    /// @brief the next attribute in the list
    /// should be NULL if this is the last attribute
    tree_attrib_t *next;
} tree_attrib_t;

CT_TREE_API tree_attrib_t *tree_attrib_add(tree_attrib_t *root, IN_NOTNULL const tree_t *schema, IN_NOTNULL const vector_t *args);

CT_TREE_API tree_t *tree_decl_attrib(const node_t *node, const char *name, vector_t *params, bitset_t mask);

CT_END_API
