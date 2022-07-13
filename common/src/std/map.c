#include "std/map.h"
#include "base/memory.h"
#include "base/panic.h"
#include "base/util.h"

#include "std/str.h"
#include "std/vector.h"

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
    size_t size;                      ///< the number of buckets in the toplevel
    FIELD_SIZE(size) bucket_t data[]; ///< the buckets
} map_t;

// generic map functions

static size_t sizeof_map(size_t size)
{
    return sizeof(map_t) + (size * sizeof(bucket_t));
}

static bucket_t *bucket_new(const void *key, void *value)
{
    bucket_t *entry = ctu_malloc(sizeof(bucket_t));
    entry->key = key;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

static bucket_t *get_bucket(map_t *map, size_t hash)
{
    size_t index = hash % map->size;
    bucket_t *entry = &map->data[index];
    return entry;
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
map_t *map_new(size_t size)
{
    CTASSERT(size > 0);

    map_t *map = ctu_malloc(sizeof_map(size));

    map->size = size;

    clear_keys(map->data, size);

    return map;
}

#define MAP_FOREACH_APPLY(self, item, ...)                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        for (size_t i = 0; i < self->size; i++)                                                                        \
        {                                                                                                              \
            bucket_t *item = &self->data[i];                                                                           \
            while (item && item->key)                                                                                  \
            {                                                                                                          \
                __VA_ARGS__;                                                                                           \
                item = item->next;                                                                                     \
            }                                                                                                          \
        }                                                                                                              \
    } while (0)

USE_DECL
vector_t *map_values(map_t *map)
{
    CTASSERT(map != NULL);

    vector_t *result = vector_new(map->size);

    MAP_FOREACH_APPLY(map, entry, { vector_push(&result, entry->value); });

    return result;
}

static map_entry_t *new_entry(const char *key, void *value)
{
    map_entry_t *entry = ctu_malloc(sizeof(map_entry_t));
    entry->key = key;
    entry->value = value;
    return entry;
}

USE_DECL
vector_t *map_entries(map_t *map)
{
    CTASSERT(map != NULL);

    vector_t *result = vector_new(map->size);

    MAP_FOREACH_APPLY(map, entry, { vector_push(&result, new_entry(entry->key, entry->value)); });

    return result;
}

// string key map functions

static bucket_t *map_bucket_str(map_t *map, const char *key)
{
    size_t hash = strhash(key);
    return get_bucket(map, hash);
}

static void *entry_get(const bucket_t *entry, const char *key, void *other)
{
    if (entry->key && str_equal(entry->key, key))
    {
        return entry->value;
    }

    if (entry->next)
    {
        return entry_get(entry->next, key, other);
    }

    return other;
}

USE_DECL
void *map_get_default(map_t *map, const char *key, void *other)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    bucket_t *bucket = map_bucket_str(map, key);
    return entry_get(bucket, key, other);
}

USE_DECL
void *map_get(map_t *map, const char *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    return map_get_default(map, key, NULL);
}

void map_set(map_t *map, const char *key, void *value)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    bucket_t *entry = map_bucket_str(map, key);

    while (true)
    {
        if (entry->key == NULL)
        {
            entry->key = key;
            entry->value = value;
            break;
        }

        if (str_equal(entry->key, key))
        {
            entry->value = value;
            break;
        }

        if (entry->next == NULL)
        {
            entry->next = bucket_new(key, value);
            break;
        }

        entry = entry->next;
    }
}

// ptr map functions

static bucket_t *map_bucket_ptr(map_t *map, const void *key)
{
    size_t hash = ptrhash(key);
    return get_bucket(map, hash);
}

static void *entry_get_ptr(const bucket_t *entry, const void *key, void *other)
{
    if (entry->key == key)
    {
        return entry->value;
    }

    if (entry->next)
    {
        return entry_get_ptr(entry->next, key, other);
    }

    return other;
}

void map_set_ptr(map_t *map, const void *key, void *value)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    bucket_t *entry = map_bucket_ptr(map, key);

    while (true)
    {
        if (entry->key == NULL)
        {
            entry->key = key;
            entry->value = value;
            break;
        }

        if (entry->key == key)
        {
            entry->value = value;
            break;
        }

        if (entry->next == NULL)
        {
            entry->next = bucket_new(key, value);
            break;
        }

        entry = entry->next;
    }
}

USE_DECL
void *map_get_ptr(map_t *map, const void *key)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    return map_get_default_ptr(map, key, NULL);
}

USE_DECL
void *map_get_default_ptr(map_t *map, const void *key, void *other)
{
    CTASSERT(map != NULL);
    CTASSERT(key != NULL);

    bucket_t *bucket = map_bucket_ptr(map, key);
    return entry_get_ptr(bucket, key, other);
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

void map_reset(map_t *map)
{
    CTASSERT(map != NULL);

    clear_keys(map->data, map->size);
}
