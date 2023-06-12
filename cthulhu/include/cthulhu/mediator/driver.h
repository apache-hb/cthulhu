#pragma once

#include "cthulhu/mediator/common.h"

typedef struct vector_t vector_t;

typedef struct sema_t sema_t;
typedef struct reports_t reports_t;

context_t *context_new(lifetime_t *lifetime, void *ast, hlir_t *root);

context_t *add_context(lifetime_t *lifetime, vector_t *path, context_t *mod);
context_t *get_context(lifetime_t *lifetime, vector_t *path);

// context api

void *context_get_ast(context_t *context);
hlir_t *context_get_hlir(context_t *context);
sema_t *context_get_sema(context_t *context);
reports_t *context_get_reports(context_t *context);
lifetime_t *context_get_lifetime(context_t *context);
const char *context_get_name(context_t *context);

void context_update(context_t *ctx, void *ast, hlir_t *root);