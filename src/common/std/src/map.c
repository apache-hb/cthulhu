#include "std/map.h"
#include "base/panic.h"

#include "arena/arena.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

// TODO: add quadratic probing and robin hood hashing

// 90% load factor before resizing
#define MAP_LOAD_FACTOR (90)

/**
 * a bucket in a hashmap
 */
typedef struct bucket_t
{
    const void *key;       ///< the key
    void *value;           ///< any pointer value
    struct bucket_t *next; ///< the next bucket in the chain
} bucket_t;

/**
 * a hashmap
 *
 * freeing the map will not free the keys or the values.
 * these need to be freed beforehand by the owner of the container.
 */
typedef struct map_t
{
    arena_t *arena;                   ///< the arena this map is allocated in
    typeinfo_t info;                  ///< the key info for this map

    size_t size;                      ///< the number of buckets in the toplevel
    size_t used;                      ///< the number of buckets in use
    FIELD_SIZE(size) bucket_t *data; ///< the buckets
} map_t;

// generic map functions

static bucket_t *impl_bucket_new(const void *key, void *value, arena_t *arena)
{
    bucket_t *entry = ARENA_MALLOC(sizeof(bucket_t), "bucket", NULL, arena);
    entry->key = key;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

HOTFN
static bucket_t *map_get_bucket(map_t *map, size_t hash)
{
    size_t index = hash % map->size;
    return &map->data[index];
}

HOTFN
static const bucket_t *map_get_bucket_const(const map_t *map, size_t hash)
{
    size_t index = hash % map->size;
    return &map->data[index];
}

static void clear_keys(bucket_t *buckets, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        buckets[i].key = NULL;
        buckets[i].next = NULL;
    }
}

USE_DECL
map_t *map_new(size_t size, typeinfo_t info, arena_t *arena)
{
    CTASSERT(size > 0);

    CTASSERT(info.equals != NULL);
    CTASSERT(info.hash != NULL);

    map_t *map = ARENA_MALLOC(sizeof(map_t), "map", NULL, arena);

    map->arena = arena;
    map->info = info;
    map->size = size;
    map->used = 0;
    map->data = ARENA_MALLOC(sizeof(bucket_t) * size, "buckets", map, arena);

    clear_keys(map->data, size);

    return map;
}

#define MAP_FOREACH_APPLY(self, item, ...)      \
    do                                          \
    {                                           \
        for (size_t i = 0; i < self->size; i++) \
        {                                       \
            bucket_t *item = &self->data[i];    \
            while (item && item->key)           \
            {                                   \
                __VA_ARGS__;                    \
                item = item->next;              \
            }                                   \
        }                                       \
    } while (0)

USE_DECL
vector_t *map_values(map_t *map)
{
    CTASSERT(map != NULL);

    vector_t *result = vector_new(map->size, map->arena);

    MAP_FOREACH_APPLY(map, entry, { vector_push(&result, entry->value); });

    return result;
}

static map_entry_t map_entry_new(const char *key, void *value)
{
    map_entry_t entry = {
        .key = key,
        .value = value,
    };

    return entry;
}

USE_DECL
typevec_t *map_entries(map_t *map)
{
    CTASSERT(map != NULL);

    typevec_t *result = typevec_new(sizeof(map_entry_t), map->size, map->arena);

    MAP_FOREACH_APPLY(map, entry, {
        map_entry_t item = map_entry_new(entry->key, entry->value);
        typevec_push(result, &item);
    });

    return result;
}

USE_DECL
size_t map_count(const map_t *map)
{
    CTASSERT(map != NULL);

    size_t count = 0;
    MAP_FOREACH_APPLY(map, entry, { count++; });

    return count;
}

// info functions

HOTFN
static size_t impl_key_hash(const map_t *map, const void *key)
{
    const typeinfo_t *info = &map->info;
    return info->hash(key);
}

HOTFN
static bool impl_key_equal(const map_t *map, const void *lhs, const void *rhs)
{
    const typeinfo_t *info = &map->info;
    return info->equals(lhs, rhs);
}

HOTFN
static const bucket_t *impl_bucket_get_const(const map_t *map, const void *key)
{
    size_t hash = impl_key_hash(map, key);
    return map_get_bucket_const(map, hash);
}

HOTFN
static bucket_t *impl_bucket_get(map_t *map, const void *key)
{
    size_t hash = impl_key_hash(map, key);
    return map_get_bucket(map, hash);
}

static bool impl_entry_exists(const map_t *map, const bucket_t *bucket, const void *key)
{
    CTASSERT(bucket != NULL);

    if (impl_key_equal(map, bucket->key, key))
        return true;

    if (bucket->next != NULL)
        return impl_entry_exists(map, bucket->next, key);

    return false;
}

// delete a bucket and reparent the next bucket
static void impl_delete_bucket(bucket_t *previous, bucket_t *entry)
{
    CTASSERT(entry != NULL);

    entry->key = NULL;
    entry->value = NULL;

    if (previous != NULL)
    {
        previous->next = entry->next;
    }
}

static bool impl_insert_into_bucket(map_t *map, bucket_t *bucket, const void *key, void *value)
{
    if (bucket->key == NULL)
    {
        // track this as a new entry
        map->used += 1;

        bucket->key = key;
        bucket->value = value;
        return true;
    }

    if (impl_key_equal(map, bucket->key, key))
    {
        bucket->value = value;
        return true;
    }

    return false;
}

HOTFN
static void impl_resize(map_t *map, size_t new_size)
{
    bucket_t *old_data = map->data;
    size_t old_size = map->size;

    // TODO: maybe resize the old data instead of allocating new data
    map->size = new_size;
    map->used = 0;
    map->data = ARENA_MALLOC(sizeof(bucket_t) * new_size, "buckets", map, map->arena);
    clear_keys(map->data, new_size);

    for (size_t i = 0; i < old_size; i++)
    {
        bucket_t *entry = &old_data[i];
        while (entry != NULL)
        {
            if (entry->key != NULL)
            {
                map_set(map, entry->key, entry->value);
            }

            entry = entry->next;
        }
    }

    arena_free(old_data, sizeof(bucket_t) * old_size, map->arena);
}

USE_DECL HOTFN
void map_set(map_t *map, const void *key, void *value)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    // if we are over the load factor, resize the map
    if ((map->used * 100 / map->size) > MAP_LOAD_FACTOR)
        impl_resize(map, map->size * 2);

    // follow the chain
    bucket_t *bucket = impl_bucket_get(map, key);
    while (true)
    {
        if (impl_insert_into_bucket(map, bucket, key, value))
            return;

        if (bucket->next == NULL)
        {
            map->used += 1;

            bucket->next = impl_bucket_new(key, value, map->arena);
            ARENA_REPARENT(bucket->next, bucket, map->arena);
            return;
        }

        bucket = bucket->next;
    }
}

USE_DECL HOTFN
void *map_get(const map_t *map, const void *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    return map_get_default(map, key, NULL);
}

USE_DECL HOTFN
void *map_get_default(const map_t *map, const void *key, void *other)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    const bucket_t *bucket = impl_bucket_get_const(map, key);
    while (bucket != NULL)
    {
        if (bucket->key != NULL && impl_key_equal(map, bucket->key, key))
            return bucket->value;

        bucket = bucket->next;
    }

    return other;
}

USE_DECL
bool map_contains(const map_t *map, const void *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    const bucket_t *bucket = impl_bucket_get_const(map, key);
    return impl_entry_exists(map, bucket, key);
}

USE_DECL
void map_delete(map_t *map, const void *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    bucket_t *entry = impl_bucket_get(map, key);
    bucket_t *previous = entry;

    while (entry != NULL)
    {
        if (entry->key != NULL && impl_key_equal(map, entry->key, key))
        {
            impl_delete_bucket(previous, entry);
            break;
        }

        previous = entry;
        entry = entry->next;
    }
}

USE_DECL
void map_reset(map_t *map)
{
    CTASSERT(map != NULL);

    map->used = 0;
    clear_keys(map->data, map->size);
}

// iteration

PUREFN
static bucket_t *next_in_chain(bucket_t *entry)
{
    if (entry == NULL || entry->key == NULL)
    {
        return NULL;
    }

    while (entry->next != NULL)
    {
        entry = entry->next;

        if (entry->key != NULL)
        {
            return entry;
        }
    }

    return NULL;
}

/**
 * @brief get the next bucket for an iterator
 *
 * @param map the map being iterated
 * @param index the current toplevel bucket index
 * @param previous the previous bucket that was returned
 * @return bucket_t* the next bucket or NULL if there are no more buckets
 */
PUREFN
static bucket_t *find_next_bucket(map_t *map, size_t *index, bucket_t *previous)
{
    bucket_t *entry = next_in_chain(previous);
    if (entry != NULL)
    {
        return entry;
    }

    size_t i = *index;

    while (i < map->size)
    {
        entry = &map->data[i++];
        if (entry->key != NULL)
        {
            *index = i;
            return entry;
        }

        entry = next_in_chain(entry);
        if (entry != NULL)
        {
            *index = i;
            return entry;
        }
    }

    return NULL;
}

USE_DECL
map_iter_t map_iter(map_t *map)
{
    CTASSERT(map != NULL);

    size_t index = 0;

    bucket_t *bucket = find_next_bucket(map, &index, NULL);
    bucket_t *next = find_next_bucket(map, &index, bucket);

    map_iter_t iter = {
        .map = map,
        .index = index,
        .bucket = bucket,
        .next = next,
    };

    return iter;
}

USE_DECL
map_entry_t map_next(map_iter_t *iter)
{
    CTASSERT(iter != NULL);

    map_entry_t entry = {
        iter->bucket->key,
        iter->bucket->value,
    };

    iter->bucket = iter->next;
    iter->next = find_next_bucket(iter->map, &iter->index, iter->bucket);

    return entry;
}

USE_DECL
bool map_next_ex(map_iter_t *iter, const void **key, void **value)
{
    CTASSERT(iter != NULL);
    CTASSERT(key != NULL);
    CTASSERT(value != NULL);

    bool has_next = map_has_next(iter);
    if (!has_next)
        return false;

    map_entry_t entry = map_next(iter);
    *key = entry.key;
    *value = entry.value;
    return true;
}

USE_DECL
bool map_has_next(const map_iter_t *iter)
{
    CTASSERT(iter != NULL);

    return iter->bucket != NULL;
}
