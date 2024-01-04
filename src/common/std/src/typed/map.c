#include "std/typed/map.h"

typemap_t *typemap_new(typemap_info_t info, size_t len, arena_t *arena);
typemap_t *typemap_optimal(typemap_info_t info, size_t len, arena_t *arena);

void *typemap_get(typemap_t *map, const void *key);
void *typemap_get_default(typemap_t *map, const void *key, void *other);

const void *typemap_set(typemap_t *map, const void *key, const void *value);

bool typemap_contains(typemap_t *map, const void *key);

bool typemap_erase(typemap_t *map, const void *key);

void typemap_reset(typemap_t *map);
