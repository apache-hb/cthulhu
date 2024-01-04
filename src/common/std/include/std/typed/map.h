#pragma once

#include "core/compiler.h"

#include <stdbool.h>

BEGIN_API

typedef struct arena_t arena_t;
typedef struct typeinfo_t typeinfo_t;
typedef struct typemap_t typemap_t;

typedef struct typemap_info_t
{
    const typeinfo_t *key;
    const typeinfo_t *value;
} typemap_info_t;

typemap_t *typemap_new(typemap_info_t info, size_t len, arena_t *arena);
typemap_t *typemap_optimal(typemap_info_t info, size_t len, arena_t *arena);

void *typemap_get(typemap_t *map, const void *key);
void *typemap_get_default(typemap_t *map, const void *key, void *other);

const void *typemap_set(typemap_t *map, const void *key, const void *value);

bool typemap_contains(typemap_t *map, const void *key);

bool typemap_erase(typemap_t *map, const void *key);

void typemap_reset(typemap_t *map);

END_API
