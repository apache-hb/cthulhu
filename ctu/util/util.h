#pragma once

#include <stddef.h>

#define MAX(L, R) ((L) > (R) ? (L) : (R)) 
#define MIN(L, R) ((L) < (R) ? (L) : (R)) 

void *ctu_malloc(size_t size);
void *ctu_realloc(void *ptr, size_t size);
void ctu_free(void *ptr);

typedef struct entry_t {
    const char *id;
    void *data;
    struct entry_t *next;
} entry_t;

typedef struct {
    size_t size;
    entry_t *data;
} map_t;

map_t *new_map(size_t size);
void *map_get(map_t *map, const char *id);
void map_put(map_t *map, const char *id, void *data);
