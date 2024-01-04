#include "std/typed/set.h"

#include "base/panic.h"

#include "memory/arena.h"

#include "std/typed/typeinfo.h"

#include <string.h>

typedef struct item_t
{
    struct item_t *next;
    char key[];
} item_t;

typedef struct typeset_t
{
    arena_t *arena;
    const typeinfo_t *info;

    FIELD_RANGE(>, 0) size_t size;
    char items[];
} typeset_t;

static size_t item_size(size_t key_size)
{
    return sizeof(item_t) + key_size;
}

static size_t typeset_size(size_t size, size_t type_size)
{
    return sizeof(typeset_t) + ((item_size(type_size)) * size);
}

static item_t *item_at(typeset_t *set, size_t index)
{
    size_t offset = item_size(set->info->size) * index;

    return (item_t *)(set->items + offset);
}

static item_t *item_new(size_t key_size, arena_t *arena)
{
    item_t *item = ARENA_MALLOC(arena, item_size(key_size), "item", NULL);
    item->next = NULL;
    return item;
}

static item_t *get_bucket_ptr(typeset_t *set, const void *value)
{
    size_t hash = set->info->fn_hash(value);
    size_t index = hash % set->size;

    return item_at(set, index);
}

static bool item_equals(typeset_t *set, item_t *item, const void *value)
{
    return set->info->fn_compare(item->key, value) == eStrongOrderingEqual;
}

typeset_t *typeset_new(const typeinfo_t *info, size_t len, arena_t *arena)
{
    CTASSERT(info != NULL);
    CTASSERT(info->size > 0);
    CTASSERT(info->fn_compare != NULL);
    CTASSERT(info->fn_hash != NULL);

    CTASSERT(len > 0);

    typeset_t *set = ARENA_MALLOC(arena, typeset_size(len, info->size), "typeset", NULL);
    set->arena = arena;
    set->info = info;
    set->size = len;

    typeset_reset(set);

    return set;
}

const void *typeset_add(typeset_t *set, const void *value)
{
    CTASSERT(set != NULL);
    CTASSERT(value != NULL);

    item_t *bucket = get_bucket_ptr(set, value);

    if (item_equals(set, bucket, set->info->empty))
    {
        memcpy(bucket->key, value, set->info->size);
        return bucket->key;
    }

    for (item_t *item = bucket->next; item != NULL; item = item->next)
    {
        if (item_equals(set, item, value))
        {
            return item->key;
        }
    }

    item_t *item = item_new(set->info->size, set->arena);
    memcpy(item->key, value, set->info->size);
    item->next = bucket->next;
    bucket->next = item;

    return item->key;
}

bool typeset_contains(typeset_t *set, const void *value)
{
    CTASSERT(set != NULL);
    CTASSERT(value != NULL);

    item_t *bucket = get_bucket_ptr(set, value);

    for (item_t *item = bucket->next; item != NULL; item = item->next)
    {
        if (item_equals(set, item, value))
        {
            return true;
        }
    }

    return false;
}

void typeset_reset(typeset_t *set)
{
    CTASSERT(set != NULL);

    for (size_t i = 0; i < set->size; i++)
    {
        item_t *item = item_at(set, i);
        item->next = NULL;
        memcpy(item->key, set->info->empty, set->info->size);
    }
}
