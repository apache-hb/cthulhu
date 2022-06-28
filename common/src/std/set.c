#include "std/set.h"
#include "base/util.h"
#include "std/str.h"

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
    size_t size;                     ///< the number of buckets
    FIELD_SIZE(size) item_t items[]; ///< the buckets
} set_t;

NODISCARD
static size_t set_size(size_t size)
{
    return sizeof(set_t) + (sizeof(item_t) * size);
}

static item_t *item_new(const char *key)
{
    item_t *item = ctu_malloc(sizeof(item_t));
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
set_t *set_new(size_t size)
{
    set_t *set = ctu_malloc(set_size(size));
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
            item->next = item_new(key);
            return key;
        }
    }
}

USE_DECL
bool set_contains(set_t *set, const char *key)
{
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
            item->next = item_new(key);
            return key;
        }
    }
}

USE_DECL
bool set_contains_ptr(set_t *set, const void *key)
{
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
    for (size_t i = 0; i < set->size; i++)
    {
        item_t *item = &set->items[i];
        while (item->next != NULL)
        {
            item_t *next = item->next;
            item->next = next->next;
            ctu_free(next);
        }
        item->next = NULL;
        item->key = NULL;
    }
}
