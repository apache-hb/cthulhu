#include "std/typed/set.h"

#include "base/panic.h"
#include "base/memory.h"

typedef struct typeset_item_t {
    const void *key;
    typeset_item_t *next;
} typeset_item_t;

typedef struct typeset_t {
    const typeset_info_t *info;

    size_t size;
    FIELD_SIZE(size) typeset_item_t items[];
} typeset_t;

static typeset_item_t *item_create(size_t typeSize)
{
    size_t size = sizeof(typeset_item_t) + typeSize;
    typeset_item_t *item = ctu_malloc(size);
    item->next = NULL;
    return item;
}

static typeset_item_t *item_at(typeset_t *set, size_t index)
{
    return &set->items[index];
}

static typeset_item_t *item_from_hash(typeset_t *set, const void *item)
{
    size_t hash = set->info->fnHash(item);
    size_t index = hash % set->size;
    return item_at(set, index);
}

static typeset_t *typeset_create(size_t size, const typeset_info_t *info)
{
    CTASSERT(info->typeSize > 0);
    CTASSERT(info->fnHash != NULL);
    CTASSERT(info->fnEqual != NULL);

    typeset_t *set = ctu_malloc(sizeof(typeset_t) + (size * sizeof(typeset_item_t)));
    set->info = info;
    set->size = size;

    typeset_reset(set);

    return set;
}

USE_DECL
typeset_t *typeset_new(size_t size, const typeset_info_t *info)
{
    CTASSERT(size > 0);
    CTASSERT(info != NULL);

    return typeset_create(size, info);
}

USE_DECL
const void *typeset_add(typeset_t *set, const void *key)
{
    CTASSERT(set != NULL);
    CTASSERT(key != NULL);

    typeset_item_t *item = item_from_hash(set, key);

    while (true)
    {
        if (item->key == NULL)
        {
            item->key = key;
            return key;
        }
        else if (set->info->fnEqual(item->key, key))
        {
            return item->key;
        }
        else if (item->next == NULL)
        {
            item->next = item_create(set->info->typeSize);
            item = item->next;
        }
        else
        {
            item = item->next;
        }
    }
}

USE_DECL
bool typeset_contains(typeset_t *set, const void *key)
{
    CTASSERT(set != NULL);
    CTASSERT(key != NULL);

    typeset_item_t *item = item_from_hash(set, key);

    while (true)
    {
        if (item->key == NULL)
        {
            return false;
        }

        if (set->info->fnEqual(item->key, key))
        {
            return true;
        }

        if (item->next == NULL)
        {
            return false;
        }
        else
        {
            item = item->next;
        }
    }

}

USE_DECL
bool typeset_empty(typeset_t *set)
{
    typeset_iter_t iter = typeset_iter(set);
    while (typeset_has_next(&iter))
    {
        const void *key = typeset_next(&iter);
        if (key != NULL)
        {
            return false;
        }
    }

    return true;
}

void typeset_reset(typeset_t *set)
{
    CTASSERT(set != NULL);

    for (size_t i = 0; i < set->size; i++)
    {
        typeset_item_t *item = item_at(set, i);
        item->key = NULL;
        item->next = NULL;
    }
}

///
/// iterator
///

static typeset_item_t *typeset_next_in_chain(typeset_item_t *item)
{
    if (item == NULL || item->next == NULL)
    {
        return NULL;
    }

    return item->next;
}

static typeset_item_t *typeset_next_item(typeset_t *set, size_t *index, typeset_item_t *previous)
{
    typeset_item_t *item = typeset_next_in_chain(previous);
    if (item != NULL)
    {
        return item;
    }

    size_t i = *index;

    while (i < set->size)
    {
        typeset_item_t *entry = item_at(set, i++);
        if (entry->key != NULL)
        {
            *index = i;
            return entry;
        }

        typeset_item_t *next = typeset_next_in_chain(entry);
        if (next != NULL)
        {
            *index = i;
            return next;
        }
    }

    return NULL;
}

USE_DECL
typeset_iter_t typeset_iter(typeset_t *set)
{
    CTASSERT(set != NULL);

    size_t index = 0;

    typeset_item_t *current = typeset_next_item(set, &index, NULL);
    typeset_item_t *next = typeset_next_item(set, &index, current);

    typeset_iter_t iter = {
        .set = set,
        .index = 0,

        .current = current,
        .next = next,
    };

    return iter;
}

USE_DECL
const void *typeset_next(typeset_iter_t *iter)
{
    CTASSERT(iter != NULL);

    const void *key = iter->current->key;

    iter->current = iter->next;
    iter->next = typeset_next_item(iter->set, &iter->index, iter->current);

    return key;
}

USE_DECL
bool typeset_has_next(typeset_iter_t *iter)
{
    CTASSERT(iter != NULL);

    return iter->current != NULL;
}
