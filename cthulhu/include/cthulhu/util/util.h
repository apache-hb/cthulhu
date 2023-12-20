#pragma once

#include "core/compiler.h"

#include <stdbool.h>

#include <gmp.h>

typedef struct tree_t tree_t;
typedef struct vector_t vector_t;
typedef struct node_t node_t;

BEGIN_API

/// @defgroup RuntimeUtil Driver utility functions
/// @brief Utility functions for the runtime
/// @ingroup Runtime
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
void *util_select_decl(tree_t *sema, const size_t *tags, size_t len, const char *name);

typedef struct decl_search_t {
    const size_t *local_tags;
    size_t local_count;

    const size_t *global_tags;
    size_t global_count;

    const size_t *decl_tags;
    size_t decl_count;
} decl_search_t;

/// @brief search for a namespace given a path, ignoring the last element in the path
///
/// @param sema the sema context
/// @param search the search options
/// @param node the node to report errors on
/// @param path the path to search
/// @param[out] is_imported whether the namespace was imported
///
/// @return the namespace if found, an error otherwise
tree_t *util_search_namespace(tree_t *sema, const decl_search_t *search, const node_t *node, vector_t *path, bool *is_imported);

/// @brief search for a decl given a path
///
/// @param sema the sema context
/// @param search the search options
/// @param node the node to report errors on
/// @param path the path to search
///
/// @return tree_t* the decl if found, an error otherwise
tree_t *util_search_path(tree_t *sema, const decl_search_t *search, const node_t *node, vector_t *path);

/// @brief search for a decl inside a module
///
/// @param sema the sema context
/// @param search the search options
/// @param node the node to report errors on
/// @param mod the module to search in
/// @param name the name of the decl
///
/// @return tree_t*
tree_t *util_search_qualified(tree_t *sema, const decl_search_t *search, const node_t *node, const char *mod, const char *name);

///
/// context
///

tree_t *util_current_module(tree_t *sema);
void util_set_current_module(tree_t *sema, tree_t *module);

tree_t *util_current_symbol(tree_t *sema);
void util_set_current_symbol(tree_t *sema, tree_t *symbol);

/// @brief evaluate a digit expression
///
/// @param[out] value the value to set
/// @param expr the expression to evaluate
///
/// @return true if the expression was evaluated, false otherwise
bool util_eval_digit(mpz_t value, const tree_t *expr);

tree_t *util_create_string(tree_t *sema, const node_t *node, tree_t *letter, const char *text, size_t length);

/// @}

END_API
