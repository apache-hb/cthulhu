#include "std/set.h"
#include "std/str.h"

#include "base/memory.h"
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
    alloc_t *alloc;

    size_t size;                     ///< the number of buckets
    FIELD_SIZE(size) item_t items[]; ///< the buckets
} set_t;

NODISCARD
static size_t set_size(size_t size)
{
    return sizeof(set_t) + (sizeof(item_t) * size);
}

static item_t *item_new(alloc_t *alloc, const char *key)
{
    item_t *item = arena_malloc(alloc, sizeof(item_t), "set-item");
    item->key = key;
    item->next = NULL;
    return item;
}

static item_t *get_bucket(set_t *set, const char *key)
{
    size_t hash = strhash(key);
    size_t index = hash % set->size;
    return &set->items[index];
}

static item_t *get_bucket_ptr(set_t *set, const void *key)
{
    size_t hash = ptrhash(key);
    size_t index = hash % set->size;
    return &set->items[index];
}

USE_DECL
set_t *set_new(size_t size, alloc_t *alloc, const char *name)
{
    CTASSERT(size > 0);
    CTASSERT(alloc != NULL);

    set_t *set = arena_malloc(alloc, set_size(size), name);
    set->alloc = alloc;
    set->size = size;

    for (size_t i = 0; i < size; i++)
    {
        set->items[i].key = NULL;
        set->items[i].next = NULL;
    }

    return set;
}

const char *set_add(set_t *set, const char *key)
{
    CTASSERT(set != NULL);
    CTASSERT(key != NULL);

    item_t *item = get_bucket(set, key);

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
            item->next = item_new(set->alloc, key);
            return key;
        }
    }
}

USE_DECL
bool set_contains(set_t *set, const char *key)
{
    CTASSERT(set != NULL);
    CTASSERT(key != NULL);

    item_t *item = get_bucket(set, key);

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
            item->next = item_new(set->alloc, key);
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

void set_reset(set_t *set)
{
    CTASSERT(set != NULL);

    for (size_t i = 0; i < set->size; i++)
    {
        item_t *item = &set->items[i];
        while (item->next != NULL)
        {
            item_t *next = item->next;
            item->next = next->next;
            arena_free(set->alloc, next, sizeof(item_t));
        }
        item->next = NULL;
        item->key = NULL;
    }
}
