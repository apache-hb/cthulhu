#pragma once

#include <ctu_util_api.h>

#include "core/compiler.h"

#include <stdbool.h>

#include <gmp.h>

typedef struct tree_t tree_t;
typedef struct vector_t vector_t;
typedef struct node_t node_t;

CT_BEGIN_API

/// @defgroup runtime_util Driver utility functions
/// @brief Utility functions for the runtime
/// @ingroup runtime
/// @{

///
/// query helpers
///

/// @brief search for a declaration by name in a set of tags
///
/// @param sema the sema context
/// @param tags the tags ids to search in
/// @param len the count of @p tags
/// @param name the name of the decl
///
/// @return the decl if found, an error otherwise
CT_UTIL_API void *util_select_decl(tree_t *sema, const size_t *tags, size_t len, const char *name);

typedef struct decl_search_t {
    const size_t *module_tags;
    size_t module_count;

    const size_t *decl_tags;
    size_t decl_count;
} decl_search_t;

/// @brief search for a namespace given a path, ignoring the last element in the path
///
/// @param sema the sema context
/// @param search the search options
/// @param node the node to report errors on
/// @param path the path to search
/// @param is_imported whether the namespace was imported
///
/// @return the namespace if found, an error otherwise
CT_UTIL_API tree_t *util_search_namespace(tree_t *sema, const decl_search_t *search, const node_t *node, vector_t *path, bool *is_imported);

/// @brief search for a decl given a path
///
/// @param sema the sema context
/// @param search the search options
/// @param node the node to report errors on
/// @param path the path to search
///
/// @return tree_t* the decl if found, an error otherwise
CT_UTIL_API tree_t *util_search_path(tree_t *sema, const decl_search_t *search, const node_t *node, vector_t *path);

/// @brief search for a decl inside a module
///
/// @param sema the sema context
/// @param search the search options
/// @param node the node to report errors on
/// @param mod the module to search in
/// @param name the name of the decl
///
/// @return tree_t*
CT_UTIL_API tree_t *util_search_qualified(tree_t *sema, const decl_search_t *search, const node_t *node, const char *mod, const char *name);

/// @brief evaluate a digit expression
///
/// @param value the value to set
/// @param expr the expression to evaluate
///
/// @return true if the expression was evaluated, false otherwise
CT_UTIL_API bool util_eval_digit(mpz_t value, const tree_t *expr);

/// @}

CT_END_API
