#pragma once

#include "cthulhu/runtime/runtime.h"

#include <stddef.h>

BEGIN_API

typedef struct vector_t vector_t;

/// @ingroup mediator
/// @{

/// @brief make a new precompiled context
///
/// @param handle the driver to use
/// @param root the tree of the context
///
/// @return the new context
CT_RUNTIME_API context_t *compiled_new(IN_NOTNULL driver_t *handle, IN_NOTNULL tree_t *root);

/// @brief make a new context
///
/// @param handle the driver to use
/// @param name the name of the context
/// @param ast the ast of the context
/// @param root the tree of the context
///
/// @return the new context
CT_RUNTIME_API context_t *context_new(IN_NOTNULL driver_t *handle, const char *name, void *ast, tree_t *root);

/// @brief add a new context to the lifetime
///
/// @param lifetime the lifetime object
/// @param path the path to the context
/// @param mod the context to add
///
/// @return the new context
CT_RUNTIME_API context_t *add_context(IN_NOTNULL lifetime_t *lifetime, IN_NOTNULL vector_t *path, IN_NOTNULL context_t *mod);

/// @brief get a context from the lifetime
///
/// @param lifetime the lifetime object
/// @param path the path to the context
///
/// @return the new context
CT_RUNTIME_API context_t *get_context(IN_NOTNULL lifetime_t *lifetime, IN_NOTNULL vector_t *path);

/// @brief get the lifetime of a driver
///
/// @param handle the driver to get the lifetime of
///
/// @return the lifetime of @p handle
CT_RUNTIME_API lifetime_t *handle_get_lifetime(IN_NOTNULL driver_t *handle);

/// @brief get the ast of a context
///
/// @param context the context to get the ast of
///
/// @return the ast of @p context
CT_RUNTIME_API void *context_get_ast(IN_NOTNULL const context_t *context);

/// @brief get the tree of a context
///
/// @param context the context to get the tree of
///
/// @return the tree of @p context
CT_RUNTIME_API tree_t *context_get_module(IN_NOTNULL const context_t *context);

/// @brief get the lifetime of a context
///
/// @param context the context to get the lifetime of
///
/// @return the lifetime of @p context
CT_RUNTIME_API lifetime_t *context_get_lifetime(IN_NOTNULL const context_t *context);

/// @brief get the name of a context
///
/// @param context the context to get the name of
CT_RUNTIME_API const char *context_get_name(IN_NOTNULL const context_t *context);

/// @brief update a context with a new ast and tree
///
/// @param ctx the context to update
/// @param ast the new ast
/// @param root the new tree
CT_RUNTIME_API void context_update(IN_NOTNULL context_t *ctx, void *ast, tree_t *root);

/// @brief create a new sema context from a lifetime
///
/// @param lifetime the lifetime object
/// @param name the name of the context
/// @param len the length of the name
/// @param sizes the sizes of the sema objects
///
/// @return the new sema context
CT_RUNTIME_API tree_t *lifetime_sema_new(IN_NOTNULL lifetime_t *lifetime, IN_STRING const char *name, IN_RANGE(>, 0) size_t len, IN_READS(len) const size_t *sizes);

/// @}

END_API
