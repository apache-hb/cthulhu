// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "cthulhu/tree/ops.h"

#include "core/analyze.h"

#include <stddef.h>
#include <stdbool.h>

CT_BEGIN_API

typedef struct arena_t arena_t;
typedef struct node_t node_t;
typedef struct vector_t vector_t;

typedef struct tree_context_t tree_context_t;
typedef struct tree_t tree_t;

/// @brief storage for a value
typedef struct tree_storage_t
{
    /// @brief the underlying storage type
    const tree_t *storage;

    /// @brief the count of elements
    size_t length;

    /// @brief the qualifiers for the storage
    tree_quals_t quals;
} tree_storage_t;

///
/// queries
///

CT_TREE_API void tree_set_qualifiers(IN_NOTNULL tree_t *tree, tree_quals_t qualifiers);
CT_TREE_API tree_quals_t tree_get_qualifiers(IN_NOTNULL const tree_t *tree);

CT_TREE_API void tree_set_storage(IN_NOTNULL tree_t *tree, tree_storage_t storage);
CT_TREE_API tree_storage_t tree_get_storage(IN_NOTNULL const tree_t *tree);

CT_TREE_API void tree_set_eval(IN_NOTNULL tree_t *tree, eval_model_t model);
CT_TREE_API eval_model_t tree_get_eval(IN_NOTNULL const tree_t *tree);

CT_PUREFN
CT_TREE_API const node_t *tree_get_node(IN_NOTNULL const tree_t *tree);

CT_PUREFN
CT_TREE_API const char *tree_get_name(IN_NOTNULL const tree_t *tree);

CT_PUREFN
CT_TREE_API const char *tree_get_user_name(IN_NOTNULL const tree_t *tree);

CT_PUREFN
CT_TREE_API bool tree_is_symbol_anonymous(IN_NOTNULL const tree_t *tree);

CT_PUREFN
CT_TREE_API const tree_t *tree_get_type(IN_NOTNULL const tree_t *tree);

CT_PUREFN
CT_TREE_API tree_kind_t tree_get_kind(IN_NOTNULL const tree_t *tree);

CT_PUREFN
CT_TREE_API bool tree_is(const tree_t *self, tree_kind_t kind);

///
/// declarations
///

CT_END_API
