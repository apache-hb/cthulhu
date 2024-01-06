#include "std/typed/map.h"
#include "base/panic.h"
#include "core/macros.h"
#include "memory/arena.h"
#include <string.h>

#define MAP_INACTIVE SIZE_MAX
#define MAP_LOAD_RATIO 80

// typemap_entry_t is a key-value pair with no definition
// since its generic, and we construct it from typeinfo
// there is no definition anywhere. the things i do to avoid
// adding a dependancy on C++...


typedef struct index_t
{
    size_t slot;
    size_t next;
} index_t;

typedef struct typemap_t
{
    arena_t *arena;
    const typeinfo_t *key;
    const typeinfo_t *value;

    index_t *indices;
    typemap_entry_t *pairs;

    // treated as x/100
    uint32_t load_ratio;
    size_t mask;
    size_t capacity;
    size_t filled;
    size_t last;

    size_t tail;
} typemap_t;


static size_t entry_size(const typemap_t *map)
{
    size_t key_size = map->key->size;
    size_t value_size = map->value->size;

    // handle alignment
    return MAX(key_size, MIN(value_size, sizeof(max_align_t))) + value_size;
}

static size_t multiply_by_ratio(size_t value, uint32_t ratio)
{
    value *= 100;
    value /= ratio;
    return value;
}

static size_t map_keysize(const typemap_t *map)
{
    return map->key->size;
}

static size_t map_max_size(const typemap_t *map)
{
    return (1 << (map_keysize(map) * 8 - 1));
}

static void set_max_load_ratio(typemap_t *map, size_t load_ratio)
{
    if (99 > load_ratio && load_ratio > 20)
    {
        map->load_ratio = load_ratio;
    }
}

static void *alloc_buckets(const typemap_t *map, size_t buckets)
{
    size_t size = map->value->size;
    return ARENA_MALLOC(map->arena, buckets * info->size, "buckets", map);
}

static void map_rebuild(typemap_t *map, size_t buckets)
{
    size_t new_bucket_count = multiply_by_ratio(buckets, map->load_ratio) + 4;

    // TODO: realloc instead
    typemap_entry_t *new_pairs = alloc_buckets(new_bucket_count, map->value, map->arena);
    memcpy(new_pairs, map->pairs, map->filled * map->value->size);

    map->pairs = new_pairs;
}

static void map_rehash(typemap_t *map, size_t buckets)
{
    if (buckets < map->filled)
        return;

    CTASSERTF(buckets < map_max_size(map), "map cannot be resized to %zu buckets", buckets);
    size_t new_buckets = map->filled > (1 << 16) ? (1 << 16) : 4;
    while (new_buckets < buckets)
        new_buckets *= 2;

    map->last = map->mask / 4;
    map->mask = new_buckets - 1;
    map->capacity = new_buckets;

    map_rebuild(map, new_buckets);
}

typemap_t *typemap_new(map_info_t info, size_t buckets, arena_t *arena)
{
    const typeinfo_t *key = info.key;
    const typeinfo_t *value = info.value;

    CTASSERT(key != NULL);
    CTASSERT(value != NULL);

    CTASSERT(key->fn_hash != NULL);
    CTASSERT(key->fn_cmp != NULL);

    CTASSERT(key->size > 0);
    CTASSERT(value->size > 0);

    typemap_t *map = ARENA_MALLOC(arena, sizeof(typemap_t), "typemap", NULL);
    map->arena = arena;
    map->key = key;
    map->value = value;

    map->indices = NULL;
    map->pairs = NULL;

    map->load_ratio = 50;
    map->mask = 0;
    map->capacity = 0;
    map->filled = 0;
    map->last = 0;

    set_max_load_ratio(map, MAP_LOAD_RATIO);

    map_rehash(map, buckets);

    return map;
}
