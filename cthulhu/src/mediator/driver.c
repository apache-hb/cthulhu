#pragma once

#include "common.h"

#include "base/panic.h"

#include "std/str.h"

static char *path_to_string(vector_t *path)
{
    CTASSERT(path != NULL);

    return str_join(".", path);
}

context_t *v2_add_context(lifetime_t *lifetime, vector_t *path, context_t *mod)
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

context_t *v2_get_context(lifetime_t *lifetime, vector_t *path)
{
    CTASSERT(lifetime != NULL);

    char *name = path_to_string(path);

    return map_get(lifetime->modules, name);
}
