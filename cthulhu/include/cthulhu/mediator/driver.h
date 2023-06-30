#pragma once

#include "cthulhu/mediator/common.h"

typedef struct vector_t vector_t;

typedef struct reports_t reports_t;

// make a precompiled context
context_t *compiled_new(driver_t *handle, h2_t *root);

// make a context from an ast
context_t *context_new(driver_t *handle, const char *name, void *ast, h2_t *root);

context_t *add_context(lifetime_t *lifetime, vector_t *path, context_t *mod);
context_t *get_context(lifetime_t *lifetime, vector_t *path);

lifetime_t *handle_get_lifetime(driver_t *handle);

// context api

void *context_get_ast(context_t *context);
h2_t *context_get_module(context_t *context);
lifetime_t *context_get_lifetime(context_t *context);
const char *context_get_name(context_t *context);

void context_update(context_t *ctx, void *ast, h2_t *root);
