#pragma once

#include <stddef.h>
#include <stdbool.h>

#define MAX(L, R) ((L) > (R) ? (L) : (R)) 
#define MIN(L, R) ((L) < (R) ? (L) : (R)) 

#define UNUSED(x) ((void)(x))

// memory managment

void *ctu_malloc(size_t size);
void *ctu_realloc(void *ptr, size_t size);
void ctu_free(void *ptr);
char *ctu_strdup(const char *str);
void init_memory(void);

#define NEW(type) ((type *)ctu_malloc(sizeof(type)))
#define NEW_ARRAY(type, count) ((type *)ctu_malloc(sizeof(type) * (count)))
#define DELETE(ptr) ctu_free(ptr)
#define DELETE_ARRAY(ptr, len) ctu_free(ptr)

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

typedef void(*map_apply_t)(void *user, void *value);
typedef bool(*map_collect_t)(void *value);
typedef void*(*vector_apply_t)(void *value);

map_t *map_new(size_t size);
void map_delete(map_t *map);
void *map_get(map_t *map, const char *key);
void map_set(map_t *map, const char *key, void *value);
void map_apply(map_t *map, void *user, map_apply_t func);

// vector collection

typedef struct {
    size_t size;
    size_t used;
    void *data[];
} vector_t;

vector_t *vector_new(size_t size);
vector_t *vector_of(size_t len);
vector_t *vector_init(void *value);
void vector_delete(vector_t *vector);
void vector_push(vector_t **vector, void *value);
void *vector_pop(vector_t *vector);
void vector_set(vector_t *vector, size_t index, void *value);
void *vector_get(const vector_t *vector, size_t index);
size_t vector_len(const vector_t *vector);
vector_t *vector_join(const vector_t *lhs, const vector_t *rhs);
vector_t *vector_map(const vector_t *vector, vector_apply_t func);

vector_t *map_collect(map_t *map, map_collect_t filter);

#define MAP_APPLY(map, user, func) map_apply(map, user, (map_apply_t)func)
#define MAP_COLLECT(map, filter) map_collect(map, (map_collect_t)filter)
#define VECTOR_MAP(vector, func) vector_map(vector, (vector_apply_t)func)
