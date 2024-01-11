#include "std/map.h"
#include "base/panic.h"
#include "base/util.h"

#include "memory/arena.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

static size_t info_ptr_hash(const void *key) { return ptrhash(key); }
static bool info_ptr_equal(const void *lhs, const void *rhs) { return lhs == rhs; }

static size_t info_str_hash(const void *key) { return strhash(key); }
static bool info_str_equal(const void *lhs, const void *rhs) { return str_equal(lhs, rhs); }

const map_info_t kMapInfoPtr = {
    .hash = info_ptr_hash,
    .equals = info_ptr_equal,
};

const map_info_t kMapInfoString = {
    .hash = info_str_hash,
    .equals = info_str_equal,
};

/// @brief how many buckets to search for after the current bucket
/// with open addressing
#define MAP_OPEN_ADDRESS_LENGTH 2

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
    map_info_t info;                  ///< the key info for this map
    size_t size;                      ///< the number of buckets in the toplevel
    FIELD_SIZE(size) bucket_t data[]; ///< the buckets
} map_t;

// generic map functions

static size_t sizeof_map(size_t size)
{
    return sizeof(map_t) + (size * sizeof(bucket_t));
}

static bucket_t *bucket_new(const void *key, void *value, arena_t *arena)
{
    bucket_t *entry = ARENA_MALLOC(arena, sizeof(bucket_t), "bucket", NULL);
    entry->key = key;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

static size_t wrap_bucket_index(map_t *map, size_t hash)
{
    return hash % map->size;
}

static bucket_t *map_bucket_at(map_t *map, size_t index)
{
    bucket_t *entry = &map->data[index];
    return entry;
}

static bucket_t *map_get_bucket(map_t *map, size_t hash)
{
    size_t index = wrap_bucket_index(map, hash);
    return map_bucket_at(map, index);
}

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
map_t *map_new_info(size_t size, map_info_t info, arena_t *arena)
{
    CTASSERT(size > 0);

    CTASSERT(info.equals != NULL);
    CTASSERT(info.hash != NULL);

    map_t *map = ARENA_MALLOC(arena, sizeof_map(size), "map", NULL);

    map->arena = arena;
    map->info = info;
    map->size = size;

    clear_keys(map->data, size);

    return map;
}

USE_DECL
map_t *map_new(size_t size, arena_t *arena)
{
    CTASSERT(size > 0);

    map_t *map = ARENA_MALLOC(arena, sizeof_map(size), "map", NULL);

    map->arena = arena;
    map->size = size;

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

    vector_t *result = vector_new_arena(map->size, map->arena);

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
size_t map_count(map_t *map)
{
    CTASSERT(map != NULL);

    size_t count = 0;
    MAP_FOREACH_APPLY(map, entry, { count++; });

    return count;
}


// info functions

static size_t impl_key_hash(const map_t *map, const void *key)
{
    const map_info_t *info = &map->info;
    CTASSERTM(info->hash != NULL, "map is missing typeinfo, did you use map_new_info?");
    return info->hash(key);
}

static bool impl_key_equal(const map_t *map, const void *lhs, const void *rhs)
{
    const map_info_t *info = &map->info;
    CTASSERTM(info->equals != NULL, "map is missing typeinfo, did you use map_new_info?");
    return info->equals(lhs, rhs);
}

static const bucket_t *impl_bucket_get_const(const map_t *map, const void *key)
{
    size_t hash = impl_key_hash(map, key);
    return map_get_bucket_const(map, hash);
}

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

static bool impl_insert_into_bucket(const map_t *map, bucket_t *bucket, const void *key, void *value)
{
    if (bucket->key != NULL)
        return false;

    if (!impl_key_equal(map, bucket->key, key))
        return false;

    bucket->key = key;
    bucket->value = value;
    return true;
}

USE_DECL
void map_set_ex(map_t *map, const void *key, void *value)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    // follow the chain
    bucket_t *bucket = impl_bucket_get(map, key);
    while (true)
    {
        if (impl_insert_into_bucket(map, bucket, key, value))
            return;

        if (bucket->next == NULL)
        {
            bucket->next = bucket_new(key, value, map->arena);
            ARENA_REPARENT(map->arena, bucket->next, bucket);
            return;
        }

        bucket = bucket->next;
    }
}

USE_DECL
void *map_get_ex(const map_t *map, const void *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    return map_get_default_ex(map, key, NULL);
}

USE_DECL
void *map_get_default_ex(const map_t *map, const void *key, void *other)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    const bucket_t *bucket = impl_bucket_get_const(map, key);
    while (bucket != NULL)
    {
        if (impl_key_equal(map, bucket, key))
            return bucket->value;

        bucket = bucket->next;
    }

    return other;
}

USE_DECL
bool map_contains_ex(const map_t *map, const void *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    const bucket_t *bucket = impl_bucket_get_const(map, key);
    return impl_entry_exists(map, bucket, key);
}

USE_DECL
void map_delete_ex(map_t *map, const void *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    bucket_t *entry = impl_bucket_get(map, key);
    bucket_t *previous = entry;

    while (entry != NULL)
    {
        if (entry->key == NULL)
            break;

        if (impl_key_equal(map, entry->key, key))
        {
            impl_delete_bucket(previous, entry);
            break;
        }

        previous = entry;
        entry = entry->next;
    }
}







// string key map functions

static size_t map_bucket_str_index(map_t *map, const char *key)
{
    size_t hash = strhash(key);
    return wrap_bucket_index(map, hash);
}

static bucket_t *map_bucket_str(map_t *map, const char *key)
{
    size_t hash = strhash(key);
    return map_get_bucket(map, hash);
}

static bool entry_str_equal(const bucket_t *entry, const char *key)
{
    return entry->key != NULL && str_equal(entry->key, key);
}

static void *entry_get(const bucket_t *entry, const char *key, void *other)
{
    if (entry_str_equal(entry, key))
    {
        return entry->value;
    }

    if (entry->next)
    {
        return entry_get(entry->next, key, other);
    }

    return other;
}

USE_DECL HOTFN
void *map_get_default(map_t *map, const char *key, void *other)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    size_t index = map_bucket_str_index(map, key);
    bucket_t *bucket = map_bucket_at(map, index);

    // quick test the first bucket
    if (entry_str_equal(bucket, key))
        return bucket->value;

    for (size_t i = 0; i < MAP_OPEN_ADDRESS_LENGTH; i++)
    {
        size_t next_index = index + i;
        if (next_index >= map->size)
            break;

        // test next bucket
        bucket_t *next = map_bucket_at(map, next_index);
        if (entry_str_equal(next, key))
            return next->value;
    }

    // give up and walk the chain
    return entry_get(bucket, key, other);
}

USE_DECL HOTFN
void *map_get(map_t *map, const char *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    return map_get_default(map, key, NULL);
}

static bool try_insert_into_str_bucket(bucket_t *bucket, const char *key, void *value)
{
    // we can use this bucket if it has no key or has the same key
    if (bucket->key == NULL || str_equal(bucket->key, key))
    {
        bucket->key = key;
        bucket->value = value;
        return true;
    }

    return false;
}

// try inserting into the next bucket
// avoids chaining in some cases
static bool try_insert_next_str_bucket(map_t *map, size_t index, const char *key, void *value)
{
    // try the next bucket if we have it
    bucket_t *entry = map_bucket_at(map, index);
    return try_insert_into_str_bucket(entry, key, value);
}

USE_DECL HOTFN
void map_set(map_t *map, const char *key, void *value)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    size_t index = map_bucket_str_index(map, key);
    bucket_t *entry = map_bucket_at(map, index);

    // early test the first bucket
    if (try_insert_into_str_bucket(entry, key, value))
        return;

    for (size_t i = 0; i < MAP_OPEN_ADDRESS_LENGTH; i++)
    {
        size_t new_index = index + i;
        if (new_index >= map->size)
            break;

        // if we can get this key in the next bucket then we're done now
        if (try_insert_next_str_bucket(map, index, key, value))
            return;
    }

    while (true)
    {
        if (try_insert_into_str_bucket(entry, key, value))
            return;

        if (entry->next == NULL)
        {
            entry->next = bucket_new(key, value, map->arena);
            ARENA_REPARENT(map->arena, entry->next, entry);
            break;
        }

        entry = entry->next;
    }
}

// ptr map functions

static size_t map_bucket_ptr_index(map_t *map, const void *key)
{
    size_t hash = ptrhash(key);
    return wrap_bucket_index(map, hash);
}

static bucket_t *map_bucket_ptr(map_t *map, const void *key)
{
    size_t hash = ptrhash(key);
    return map_get_bucket(map, hash);
}

static bool entry_ptr_equal(const bucket_t *entry, const void *key)
{
    return entry->key == key;
}

static void *entry_get_ptr(const bucket_t *entry, const void *key, void *other)
{
    if (entry_ptr_equal(entry, key))
    {
        return entry->value;
    }

    if (entry->next)
    {
        return entry_get_ptr(entry->next, key, other);
    }

    return other;
}

static bool try_insert_into_ptr_bucket(bucket_t *bucket, const void *key, void *value)
{
    if (bucket->key == NULL || bucket->key == key)
    {
        bucket->key = key;
        bucket->value = value;
        return true;
    }

    return false;
}

static bool try_insert_next_ptr_bucket(map_t *map, size_t index, const void *key, void *value)
{
    bucket_t *bucket = map_bucket_at(map, index);
    return try_insert_into_ptr_bucket(bucket, key, value);
}

USE_DECL HOTFN
void map_set_ptr(map_t *map, const void *key, void *value)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    size_t index = map_bucket_ptr_index(map, key);
    bucket_t *entry = map_bucket_at(map, index);

    if (try_insert_into_ptr_bucket(entry, key, value))
        return;

    // open addressing
    for (size_t i = 0; i < MAP_OPEN_ADDRESS_LENGTH; i++)
    {
        size_t new_index = index + i;
        if (new_index >= map->size)
            break;

        if (try_insert_next_ptr_bucket(map, index, key, value))
            return;
    }

    while (true)
    {
        if (try_insert_into_ptr_bucket(entry, key, value))
            break;

        if (entry->next == NULL)
        {
            entry->next = bucket_new(key, value, map->arena);
            ARENA_REPARENT(map->arena, entry->next, entry);
            break;
        }

        entry = entry->next;
    }
}

USE_DECL HOTFN
void *map_get_ptr(map_t *map, const void *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    return map_get_default_ptr(map, key, NULL);
}

USE_DECL HOTFN
void *map_get_default_ptr(map_t *map, const void *key, void *other)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    size_t index = map_bucket_ptr_index(map, key);
    bucket_t *bucket = map_bucket_at(map, index);
    if (entry_ptr_equal(bucket, key))
        return bucket->value;

    for (size_t i = 0; i < MAP_OPEN_ADDRESS_LENGTH; i++)
    {
        size_t new_index = index + i;
        if (new_index >= map->size)
            break;

        bucket_t *next = map_bucket_at(map, new_index);
        if (entry_ptr_equal(next, key))
            return next->value;
    }

    // give up and follow the chain
    return entry_get_ptr(bucket, key, other);
}

static bool entry_exists(const bucket_t *entry, const char *key)
{
    if (str_equal(entry->key, key))
    {
        return true;
    }

    if (entry->next)
    {
        return entry_exists(entry->next, key);
    }

    return false;
}

static bool entry_exists_ptr(const bucket_t *entry, const void *key)
{
    if (entry->key == key)
    {
        return true;
    }

    if (entry->next)
    {
        return entry_exists_ptr(entry->next, key);
    }

    return false;
}

USE_DECL
bool map_contains(IN_NOTNULL map_t *map, IN_STRING const char *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    const bucket_t *entry = map_bucket_str(map, key);
    return entry_exists(entry, key);
}

USE_DECL
bool map_contains_ptr(IN_NOTNULL map_t *map, const void *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    const bucket_t *entry = map_bucket_ptr(map, key);
    return entry_exists_ptr(entry, key);
}

USE_DECL
void map_delete(map_t *map, const char *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    size_t hash = strhash(key);
    bucket_t *entry = map_get_bucket(map, hash);
    bucket_t *previous = entry;

    while (entry != NULL)
    {
        if (entry->key && str_equal(entry->key, key))
        {
            impl_delete_bucket(previous, entry);
            break;
        }

        previous = entry;
        entry = entry->next;
    }
}

USE_DECL
void map_delete_ptr(map_t *map, const void *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    size_t hash = ptrhash(key);
    bucket_t *entry = map_get_bucket(map, hash);
    bucket_t *previous = entry;

    while (entry != NULL)
    {
        if (entry->key == key)
        {
            impl_delete_bucket(previous, entry);
            break;
        }

        previous = entry;
        entry = entry->next;
    }
}

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
bool map_has_next(map_iter_t *iter)
{
    CTASSERT(iter != NULL);

    return iter->bucket != NULL;
}

USE_DECL
void map_reset(map_t *map)
{
    CTASSERT(map != NULL);

    clear_keys(map->data, map->size);
}
