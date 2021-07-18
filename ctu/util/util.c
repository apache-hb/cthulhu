#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "str.h"

void *ctu_malloc(size_t size) {
    return malloc(size);
}

void *ctu_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void ctu_free(void *ptr) {
    free(ptr);
}

static uint32_t hash_string(const char *str) {
    uint32_t hash = 0;
    for (size_t i = 0; i < strlen(str); i++) {
        hash = (hash << 5) - hash + str[i];
    }
    return hash;
}

static void *entry_get(entry_t *it, const char *key) {
    if (it->id == key) {
        return it->data;
    }

    if (it->next) {
        return entry_get(it->next, key);
    }

    return NULL;
}

map_t *new_map(size_t size) {
    map_t *map = ctu_malloc(sizeof(map_t));
    map->size = size;
    map->data = ctu_malloc(sizeof(entry_t) * size);
    
    for (size_t i = 0; i < size; i++) {
        map->data[i].id = NULL;
        map->data[i].next = NULL;
    }

    return map;
}

void *map_get(map_t *map, const char *id) {
    uint32_t hash = hash_string(id);
    return entry_get(&map->data[hash % map->size], id);
}

void map_put(map_t *map, const char *id, void *data) {
    uint32_t hash = hash_string(id);
    entry_t *entry = &map->data[hash % map->size];
    while (entry) {
        if (entry->id == NULL) {
            entry->id = id;
            entry->data = data;
            break;
        } else if (entry->id == id) {
            break;
        } else {
            entry->next = ctu_malloc(sizeof(entry_t));
            entry->next->id = NULL;
            entry->next->next = NULL;
            entry = entry->next;
        }
    }
}

set_t *new_set(size_t size) {
    set_t *set = ctu_malloc(sizeof(set_t));
    set->size = size;
    set->data = ctu_malloc(sizeof(item_t) * size);

    for (size_t i = 0; i < size; i++) {
        set->data[i].data = NULL;
        set->data[i].next = NULL;
    }

    return set;
}

char *set_add(set_t *set, const char *id) {
    uint32_t hash = hash_string(id);
    item_t *item = &set->data[hash % set->size];
    
    while (true) {
        if (item->data == NULL) {
            item->data = strdup(id);
            return item->data;
        } else if (strcmp(item->data, id) == 0) {
            return item->data;
        } else if (!item->next) {
            item->next = ctu_malloc(sizeof(item_t));
            item->next->data = strdup(id);
            item->next->next = NULL;
            return item->next->data;
        } else {
            item = item->next;
        }
    }
}


static set_t *pool = NULL;

void init_pool(void) {
    pool = new_set(256);
}

char *intern(const char *id) {
    return set_add(pool, id);
}
