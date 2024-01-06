#pragma once

#include "std/typed/typeinfo.h"

#include <stdbool.h>

BEGIN_API

// alot of this code is lifted from https://github.com/ktprime/emhash
// its licensed under MIT so we should be good (i hope)

typedef struct arena_t arena_t;
typedef struct typemap_t typemap_t;
typedef struct typemap_entry_t typemap_entry_t;

typedef struct map_info_t
{
    const typeinfo_t *key;
    const typeinfo_t *value;
} map_info_t;

typedef struct typemap_iter_t
{
    typemap_entry_t *entry;
} typemap_iter_t;

typemap_t *typemap_new(map_info_t info, size_t buckets, arena_t *arena);

bool typemap_contains(const typemap_t *map, const void *key);

const void *typemap_get(const typemap_t *map, const void *key);

const void *typemap_set(typemap_t *map, const void *key, const void *value);

const void *typemap_get_default(const typemap_t *map, const void *key, const void *other);

void typemap_remove(typemap_t *map, const void *key);

typemap_iter_t typemap_iter(const typemap_t *map);

bool typemap_next(const typemap_iter_t *iter, const void *key, const void *value);

END_API
