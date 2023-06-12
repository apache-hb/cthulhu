#pragma once

#include "common.h"

#include "base/panic.h"
#include "base/memory.h"

#include "std/str.h"
#include "std/vector.h"

static char *path_to_string(vector_t *path)
{
    CTASSERT(path != NULL);
    CTASSERT(vector_len(path) > 0);

    return str_join(".", path);
}

context_t *context_new(lifetime_t *lifetime, void *ast, hlir_t *root)
{
    CTASSERT(lifetime != NULL);

    context_t *self = ctu_malloc(sizeof(context_t));

    self->parent = lifetime;
    self->ast = ast;
    self->root = root;

    return self;
}

context_t *add_context(lifetime_t *lifetime, vector_t *path, context_t *mod)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(mod != NULL);

    char *name = path_to_string(path);

    context_t *old = map_get(lifetime->modules, name);
    if (old != NULL)
    {
        return old;
    }

    map_set(lifetime->modules, name, mod);
    return NULL;
}

context_t *get_context(lifetime_t *lifetime, vector_t *path)
{
    CTASSERT(lifetime != NULL);

    char *name = path_to_string(path);

    return map_get(lifetime->modules, name);
}
