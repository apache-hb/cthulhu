#include "std/set.h"
#include "std/str.h"

#include "memory/arena.h"
#include "base/panic.h"

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
    type_info_t info;
    FIELD_RANGE(>, 0) size_t size;   ///< the number of buckets
    FIELD_SIZE(size) item_t *items; ///< the buckets
} set_t;

static item_t *item_new(const char *key, arena_t *arena)
{
    item_t *item = ARENA_MALLOC(sizeof(item_t), "item", NULL, arena);
    item->key = key;
    item->next = NULL;
    return item;
}

static item_t *get_bucket_from_hash(set_t *set, size_t hash)
{
    size_t index = hash % set->size;
    return &set->items[index];
}

static void clear_items(set_t *set)
{
    for (size_t i = 0; i < set->size; i++)
    {
        item_t *item = &set->items[i];
        item->key = NULL;
        item->next = NULL;
    }
}

set_t *set_new(size_t size, type_info_t info, arena_t *arena)
{
    CTASSERT(size > 0);
    CTASSERT(arena != NULL);

    set_t *set = ARENA_MALLOC(sizeof(set_t), "set", NULL, arena);
    set->arena = arena;
    set->info = info;
    set->size = size;
    set->items = ARENA_MALLOC(sizeof(item_t) * size, "items", set, arena);

    clear_items(set);

    return set;
}

static item_t *impl_get_bucket(set_t *set, const void *key)
{
    type_info_t info = set->info;
    CTASSERT(info.hash != NULL);

    size_t hash = info.hash(key);
    return get_bucket_from_hash(set, hash);
}

static bool impl_keys_equal(const set_t *set, const void *lhs, const void *rhs)
{
    type_info_t info = set->info;
    CTASSERT(info.equals != NULL);

    return info.equals(lhs, rhs);
}

const void *set_add(set_t *set, const void *key)
{
    item_t *item = impl_get_bucket(set, key);

    while (true)
    {
        if (item->key == NULL)
        {
            item->key = key;
            return key;
        }

        if (impl_keys_equal(set, item->key, key))
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
            ARENA_REPARENT(item->next, item, set->arena);
            return key;
        }
    }

    NEVER("unreachable");
}

bool set_contains(const set_t *set, const void *key)
{
    item_t *item = impl_get_bucket((set_t*)set, key);

    while (true)
    {
        if (item->key == NULL)
        {
            return false;
        }

        if (impl_keys_equal(set, item->key, key))
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

    NEVER("unreachable");
}

void set_delete(set_t *set, const void *key)
{
    item_t *item = impl_get_bucket(set, key);

    while (true)
    {
        if (item->key == NULL)
        {
            return;
        }

        if (impl_keys_equal(set, item->key, key))
        {
            item->key = NULL;
            return;
        }

        if (item->next != NULL)
        {
            item = item->next;
        }
        else
        {
            return;
        }
    }

    NEVER("unreachable");
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
