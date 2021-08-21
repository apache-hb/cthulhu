#pragma once

#include <stddef.h>

#define MAX(L, R) ((L) > (R) ? (L) : (R)) 
#define MIN(L, R) ((L) < (R) ? (L) : (R)) 

// memory managment

void *ctu_malloc(size_t size);
void *ctu_realloc(void *ptr, size_t size);
void ctu_free(void *ptr);
char *ctu_strdup(const char *str);

// map collection

typedef struct entry_t {
    const char *key;
    void *value;
    struct entry_t *next;
} entry_t;

typedef struct {
    size_t size;
    entry_t data[];
} map_t;

map_t *map_new(size_t size);
void map_delete(map_t *map);
void *map_get(map_t *map, const char *key);
void map_set(map_t *map, const char *key, void *value);


// vector collection

typedef struct {
    size_t size;
    size_t used;
    void *data[];
} vector_t;

vector_t *vector_new(size_t size);
vector_t *vector_init(void *value);
void vector_delete(vector_t *vector);
void vector_push(vector_t **vector, void *value);
void *vector_pop(vector_t *vector);
void vector_set(vector_t *vector, size_t index, void *value);
void *vector_get(const vector_t *vector, size_t index);
size_t vector_len(const vector_t *vector);
vector_t *vector_join(const vector_t *lhs, const vector_t *rhs);
