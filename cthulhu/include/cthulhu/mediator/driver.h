#pragma once

#include "cthulhu/mediator/common.h"

typedef struct vector_t vector_t;

context_t *context_new(lifetime_t *lifetime, void *ast, hlir_t *root);

context_t *add_context(lifetime_t *lifetime, vector_t *path, context_t *mod);
context_t *get_context(lifetime_t *lifetime, vector_t *path);
