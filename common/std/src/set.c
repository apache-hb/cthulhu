#include "std/set.h"
#include "std/str.h"

#include "memory/memory.h"
#include "base/panic.h"
#include "base/util.h"

/**
 * @brief a node in a chain of set entries
 */
typedef struct item_t
{
    const void *key;     ///< the key to this bucket
    struct item_t *next; ///< the next bucket in the chain
} item_t;

typedef struct set_t
{
    arena_t *arena; ///< the arena this set is allocated in
    FIELD_RANGE(>, 0) size_t size;   ///< the number of buckets
    FIELD_SIZE(size) item_t items[]; ///< the buckets
} set_t;

NODISCARD
static size_t set_size(size_t size)
{
    return sizeof(set_t) + (sizeof(item_t) * size);
}

static item_t *item_new(const char *key, arena_t *arena)
{
    item_t *item = ARENA_MALLOC(arena, sizeof(item_t), "item", NULL);
    item->key = key;
    item->next = NULL;
    return item;
}

static item_t *get_bucket_from_hash(set_t *set, size_t hash)
{
    size_t index = hash % set->size;
    return &set->items[index];
}

static item_t *get_bucket_str(set_t *set, const char *key)
{
    size_t hash = strhash(key);
    return get_bucket_from_hash(set, hash);
}

static item_t *get_bucket_ptr(set_t *set, const void *key)
{
    size_t hash = ptrhash(key);
    return get_bucket_from_hash(set, hash);
}

USE_DECL
set_t *set_new(size_t size)
{
    CTASSERT(size > 0);

    arena_t *arena = ctu_default_alloc();
    set_t *set = ARENA_MALLOC(arena, set_size(size), "set", NULL);
    set->arena = arena;
    set->size = size;

    for (size_t i = 0; i < size; i++)
    {
        set->items[i].key = NULL;
        set->items[i].next = NULL;
    }

    return set;
}

USE_DECL
const char *set_add(set_t *set, const char *key)
{
    CTASSERT(set != NULL);
    CTASSERT(key != NULL);

    item_t *item = get_bucket_str(set, key);

    while (true)
    {
        if (item->key == NULL)
        {
            item->key = key;
            return key;
        }

        if (str_equal(item->key, key))
        {
            return item->key;
        }

        if (item->next != NULL)
        {
            item = item->next;
        }
        else
        {
            item->next = item_new(key, set->arena);
            ARENA_REPARENT(set->arena, item->next, item);
            return key;
        }
    }
}

USE_DECL
bool set_contains(set_t *set, const char *key)
{
    CTASSERT(set != NULL);
    CTASSERT(key != NULL);

    item_t *item = get_bucket_str(set, key);

    while (true)
    {
        if (item->key == NULL)
        {
            return false;
        }

        if (str_equal(item->key, key))
        {
            return true;
        }

        if (item->next != NULL)
        {
            item = item->next;
        }
        else
        {
            return false;
        }
    }
}

USE_DECL
const void *set_add_ptr(set_t *set, const void *key)
{
    CTASSERT(set != NULL);

    item_t *item = get_bucket_ptr(set, key);

    while (true)
    {
        if (item->key == NULL)
        {
            item->key = key;
            return key;
        }

        if (item->key == key)
        {
            return item->key;
        }

        if (item->next != NULL)
        {
            item = item->next;
        }
        else
        {
            item->next = item_new(key, set->arena);
            ARENA_REPARENT(set->arena, item->next, item);
            return key;
        }
    }
}

USE_DECL
bool set_contains_ptr(set_t *set, const void *key)
{
    CTASSERT(set != NULL);

    item_t *item = get_bucket_ptr(set, key);

    while (true)
    {
        if (item->key == NULL)
        {
            return false;
        }

        if (item->key == key)
        {
            return true;
        }

        if (item->next != NULL)
        {
            item = item->next;
        }
        else
        {
            return false;
        }
    }
}

USE_DECL
bool set_empty(set_t *set)
{
    set_iter_t iter = set_iter(set);
    while (set_has_next(&iter))
    {
        const void *key = set_next(&iter);
        if (key != NULL)
        {
            return false;
        }
    }

    return true;
}

USE_DECL
void set_reset(set_t *set)
{
    CTASSERT(set != NULL);

    for (size_t i = 0; i < set->size; i++)
    {
        item_t *item = &set->items[i];
        item->next = NULL;
        item->key = NULL;
    }
}

static item_t *next_in_chain(item_t *entry)
{
    if (entry == NULL || entry->key == NULL)
    {
        return NULL;
    }

    // TODO: something better than this

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
static item_t *find_next_item(set_t *set, size_t *index, item_t *previous)
{
    item_t *entry = next_in_chain(previous);
    if (entry != NULL)
    {
        return entry;
    }

    size_t i = *index;

    while (i < set->size)
    {
        entry = &set->items[i++];
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
set_iter_t set_iter(set_t *set)
{
    CTASSERT(set != NULL);

    size_t index = 0;

    item_t *current = find_next_item(set, &index, NULL);
    item_t *next = find_next_item(set, &index, current);

    set_iter_t iter = {
        .set = set,
        .index = index,
        .current = current,
        .next = next,
    };

    return iter;
}

USE_DECL
const void *set_next(set_iter_t *iter)
{
    CTASSERT(iter != NULL);

    const void *entry = iter->current->key;

    iter->current = iter->next;
    iter->next = find_next_item(iter->set, &iter->index, iter->current);

    return entry;
}

USE_DECL
bool set_has_next(set_iter_t *iter)
{
    CTASSERT(iter != NULL);

    return iter->current != NULL;
}
