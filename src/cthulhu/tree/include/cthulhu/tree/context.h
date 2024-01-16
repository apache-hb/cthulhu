#pragma once

#include "cthulhu/tree/ops.h"

#include "core/analyze.h"

#include <stdbool.h>
#include <gmp.h>

BEGIN_API

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
    quals_t quals;
} tree_storage_t;

// convert a tree node to a user friendly string
typedef char *(*tree_to_string_t)(IN_NOTNULL const tree_context_t *context, IN_NOTNULL const tree_t *tree, IN_NOTNULL arena_t *arena);

// resolve the implementation of a declaration
typedef void (*tree_resolve_t)(IN_NOTNULL tree_context_t *context, IN_NOTNULL tree_t *tree);

// resolve only the type of a declaration
typedef void (*tree_resolve_type_t)(IN_NOTNULL tree_context_t *context, IN_NOTNULL tree_t *tree);

/// @brief information about a language context
typedef struct tree_info_t
{
    /// @brief convert a tree to a user friendly string
    tree_to_string_t to_string;

    /// @brief declaration resolution
    tree_resolve_t resolve;

    /// @brief declaration type resolution
    tree_resolve_type_t resolve_type;
} tree_info_t;

CT_TREE_API tree_context_t *tree_context_new(tree_info_t info, IN_STRING const char *name, IN_NOTNULL arena_t *arena);
CT_TREE_API void tree_context_delete(OUT_PTR_INVALID tree_context_t *context);

CT_TREE_API bool tree_context_contains(IN_NOTNULL const tree_context_t *context, IN_NOTNULL const tree_t *tree);

CT_TREE_API char *tree_ctx_string(IN_NOTNULL const tree_context_t *ctx, IN_NOTNULL const tree_t *tree, IN_NOTNULL arena_t *arena);

///
/// queries
///

CT_TREE_API void tree_set_qualifiers(IN_NOTNULL tree_t *tree, quals_t qualifiers);
CT_TREE_API quals_t tree_get_qualifiers(IN_NOTNULL const tree_t *tree);

CT_TREE_API void tree_set_storage(IN_NOTNULL tree_t *tree, tree_storage_t storage);
CT_TREE_API tree_storage_t tree_get_storage(IN_NOTNULL const tree_t *tree);

CT_TREE_API const node_t *tree_get_node(IN_NOTNULL const tree_t *tree);
CT_TREE_API const char *tree_get_name(IN_NOTNULL const tree_t *tree);
CT_TREE_API const tree_t *tree_get_type(IN_NOTNULL const tree_t *tree);

///
/// types
///

CT_TREE_API tree_t *tree_type_empty_new(IN_NOTNULL tree_context_t *context, IN_NOTNULL const node_t *node, IN_STRING const char *name);

CT_TREE_API tree_t *tree_type_unit_new(IN_NOTNULL tree_context_t *context, IN_NOTNULL const node_t *node, IN_STRING const char *name);

CT_TREE_API tree_t *tree_type_bool_new(IN_NOTNULL tree_context_t *context, IN_NOTNULL const node_t *node, IN_STRING const char *name);

CT_TREE_API tree_t *tree_type_opaque_new(IN_NOTNULL tree_context_t *context, IN_NOTNULL const node_t *node, IN_STRING const char *name);

CT_TREE_API tree_t *tree_type_digit_new(IN_NOTNULL tree_context_t *context, IN_NOTNULL const node_t *node, IN_STRING const char *name, digit_t digit, sign_t sign);

CT_TREE_API tree_t *tree_type_closure_new(IN_NOTNULL tree_context_t *context, IN_NOTNULL const node_t *node, IN_STRING const char *name, IN_NOTNULL const tree_t *return_type, IN_NOTNULL vector_t *params, arity_t arity);

CT_TREE_API tree_t *tree_type_pointer_new(IN_NOTNULL tree_context_t *context, IN_NOTNULL const node_t *node, IN_STRING const char *name, IN_NOTNULL const tree_t *pointee, const tree_t *length);

CT_TREE_API tree_t *tree_type_array_new(IN_NOTNULL tree_context_t *context, IN_NOTNULL const node_t *node, IN_STRING const char *name, IN_NOTNULL const tree_t *element, IN_NOTNULL const tree_t *length);

CT_TREE_API tree_t *tree_type_reference_new(IN_NOTNULL tree_context_t *context, IN_NOTNULL const node_t *node, IN_STRING const char *name, IN_NOTNULL const tree_t *pointee);

///
/// expressions
///

CT_TREE_API tree_t *tree_expr_empty_new(IN_NOTNULL tree_context_t *context, IN_NOTNULL const node_t *node, IN_NOTNULL const tree_t *type);

CT_TREE_API tree_t *tree_expr_unit_new(IN_NOTNULL tree_context_t *context, IN_NOTNULL const node_t *node, IN_NOTNULL const tree_t *type);

///
/// declarations
///

END_API
