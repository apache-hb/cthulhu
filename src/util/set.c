#include "cthulhu/util/util.h"
#include "cthulhu/util/str.h"

static size_t set_size(size_t size) {
    return sizeof(set_t) + (sizeof(item_t) * size);
}

static item_t *item_new(const char *key) {
    item_t *item = ctu_malloc(sizeof(item_t));
    item->key = key;
    item->next = NULL;
    return item;
}

static item_t *get_bucket(set_t *set, const char *key) {
    size_t hash = strhash(key);
    size_t index = hash % set->size;
    return &set->items[index];
}

set_t *set_new(size_t size) {
    size_t bytes = set_size(size);

    set_t *set = ctu_malloc(bytes);
    set->size = size;
    for (size_t i = 0; i < size; i++) {
        set->items[i].key = NULL;
        set->items[i].next = NULL;
    }

    return set;
}

void set_delete(set_t *set) {
    for (size_t i = 0; i < set->size; i++) {
        item_t *item = &set->items[i];
        while (item->next != NULL) {
            item_t *next = item->next;
            item->next = next->next;
            ctu_free(next);
        }
    }

    ctu_free(set);
}

const char* set_add(set_t *set, const char *key) {
    item_t *item = get_bucket(set, key);

    while (true) {
        if (item->key == NULL) {
            item->key = key;
            return key;
        } else if (streq(item->key, key)) {
            return item->key;
        } else if (item->next != NULL) {
            item = item->next;
        } else {
            item->next = item_new(key);
            return key;
        }
    } 
}

bool set_contains(set_t *set, const char *key) {
    item_t *item = get_bucket(set, key);

    while (true) {
        if (item->key == NULL) {
            return false;
        } else if (streq(item->key, key)) {
            return true;
        } else if (item->next != NULL) {
            item = item->next;
        } else {
            return false;
        }
    } 
}