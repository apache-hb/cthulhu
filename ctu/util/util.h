#pragma once

#include <stddef.h>
#include <stdio.h>

#define MAX(L, R) ((L) > (R) ? (L) : (R)) 
#define MIN(L, R) ((L) < (R) ? (L) : (R)) 

void *ctu_malloc(size_t size);
void *ctu_realloc(void *ptr, size_t size);
void ctu_free(void *ptr);

FILE *ctu_open(const char *path, const char *mode);
void ctu_close(FILE *fp);

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

void map_iter(map_t *map, void (*func)(const char *id, void *data, void *arg), void *arg);

typedef struct {
    size_t size;
    size_t len;
    void **data;
} list_t;

list_t *new_list(void *init);
list_t *sized_list(size_t size);
size_t list_len(list_t *list);
void *list_at(list_t *list, size_t index);
void list_set(list_t *list, size_t index, void *data);
list_t *list_push(list_t *list, void *data);
void *list_pop(list_t *list);
void *list_first(list_t *list);
void *list_last(list_t *list);
list_t list_slice(list_t *list, size_t offset);

#define LIST_AT(T, L, I) ((T)list_at(L, I))
