#pragma once

#include "cthulhu/mediator2/common.h"

typedef struct vector_t vector_t;

typedef struct v2_language_t 
{
    const char *id;
    const char *name;

    version_info_t version;

    const char **exts;
} v2_language_t;

void *v2_add_context(lifetime_t *lifetime, vector_t *path, void *user);
void *v2_get_context(lifetime_t *lifetime, vector_t *path);
