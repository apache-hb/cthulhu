#pragma once

#include "cthulhu/mediator/common.h"

typedef struct vector_t vector_t;

context_t *v2_add_context(lifetime_t *lifetime, vector_t *path, context_t *mod);
context_t *v2_get_context(lifetime_t *lifetime, vector_t *path);
