#pragma once

#include "core/compiler.h"

#include "cthulhu/runtime/common.h"

#include <stddef.h>

BEGIN_API

typedef struct vector_t vector_t;

// make a precompiled context
context_t *compiled_new(driver_t *handle, tree_t *root);

// make a context from an ast
context_t *context_new(driver_t *handle, const char *name, void *ast, tree_t *root);

context_t *add_context(lifetime_t *lifetime, vector_t *path, context_t *mod);
context_t *get_context(lifetime_t *lifetime, vector_t *path);

lifetime_t *handle_get_lifetime(driver_t *handle);

// context api

void *context_get_ast(context_t *context);
tree_t *context_get_module(context_t *context);
lifetime_t *context_get_lifetime(context_t *context);
const char *context_get_name(context_t *context);

void context_update(context_t *ctx, void *ast, tree_t *root);

///
/// helper apis
///

tree_t *lifetime_sema_new(lifetime_t *lifetime, const char *name, size_t len, const size_t *sizes);

END_API
