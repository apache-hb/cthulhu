#include "util.h"
#include "io.h"
#include "report.h"
#include "str.h"

#include <string.h>
#include <stdlib.h>
#include <gmp.h>

#ifndef _WIN32
#   include <sys/mman.h>
#endif

#ifdef CTU_MIMALLOC
#   include <mimalloc.h>
#   define MALLOC(size) mi_malloc(size)
#   define REALLOC(ptr, size) mi_realloc(ptr, size)
#   define FREE(ptr) mi_free(ptr)
#else
#   define MALLOC(size) malloc(size)
#   define REALLOC(ptr, size) realloc(ptr, size)
#   define FREE(ptr) free(ptr)
#endif

void *ctu_malloc(size_t size) {
    return MALLOC(size);
}

void *ctu_realloc(void *ptr, size_t old, size_t size) {
    UNUSED(old);
    return REALLOC(ptr, size);
}

void ctu_free(void *ptr, size_t size) {
    UNUSED(size);
    FREE(ptr);
}

static void *ctu_gmp_malloc(size_t size) {
    return ctu_malloc(size);
}

static void *ctu_gmp_realloc(void *ptr, size_t old, size_t size) {
    return ctu_realloc(ptr, old, size);
}

static void ctu_gmp_free(void *ptr, size_t size) {
    ctu_free(ptr, size);
}

void init_memory(void) {
    mp_set_memory_functions(
        ctu_gmp_malloc, 
        ctu_gmp_realloc, 
        ctu_gmp_free
    );
}

char *ctu_strdup(const char *str) {
    size_t len = strlen(str) + 1;
    char *out = ctu_malloc(len);
    memcpy(out, str, len);
    return out;
}

void *ctu_memdup(const void *ptr, size_t size) {
    void *out = ctu_malloc(size);
    memcpy(out, ptr, size);
    return out;
}

file_t *ctu_open(const char *path, const char *mode) {
    FILE *fp = fopen(path, mode);

    if (fp == NULL) {
        return NULL;
    }

    file_t *file = ctu_malloc(sizeof(file_t));
    file->file = fp;
    file->path = path;

    return file;
}

void ctu_close(file_t *fp) {
    if (fp->file) {
        fclose(fp->file);
    }

    ctu_free(fp, sizeof(file_t));
}

size_t ctu_read(void *dst, size_t total, file_t *fp) {
    return fread(dst, 1, total, fp->file);
}

static size_t file_size(FILE *fd) {
    fseek(fd, 0, SEEK_END);
    size_t size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    return size;
}

void *ctu_mmap(file_t *fp) {
    char *text;
    size_t size = file_size(fp->file);

    printf("%s = %zu\n", fp->path, size);

#ifndef _WIN32
    int fd = fileno(fp->file);
    text = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (text == MAP_FAILED) {
        text = NULL;
    }
#else
    text = ctu_malloc(size + 1);
    fread(text, size, 1, file);
    text[size] = '\0';
#endif

    return text;
}

// map internals

/**
 * maps end with a flexible array.
 * calcuate the actual size of the map to malloc
 */
static size_t sizeof_map(map_size_t size) {
    return sizeof(map_t) + (size * sizeof(bucket_t));
}

static bucket_t *bucket_new(const char *key, void *value) { 
    bucket_t *entry = ctu_malloc(sizeof(bucket_t));
    entry->key = key;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

static void *entry_get(const bucket_t *entry, const char *key) {
    if (entry->key && streq(entry->key, key)) {
        return entry->value;
    }

    if (entry->next) {
        return entry_get(entry->next, key);
    }

    return NULL;
}

static void entry_delete(bucket_t *entry) {
    if (entry->next) {
        entry_delete(entry->next);
    }

    ctu_free(entry, sizeof(bucket_t));
}

/* find which bucket a key should be in */
static bucket_t *map_bucket(map_t *map, const char *key) {
    size_t hash = strhash(key);
    size_t index = hash % map->size;
    bucket_t *entry = &map->data[index];
    return entry;
}

// map public api

map_t *map_new(map_size_t size) {
    map_t *map = ctu_malloc(sizeof_map(size));

    map->size = size;

    /* clear out the map keys */
    for (size_t i = 0; i < size; i++) {
        map->data[i].key = NULL;
        map->data[i].next = NULL;
    }

    return map;
}

void map_delete(map_t *map) {
    /* free all entries, but dont free the toplevel ones */
    for (size_t i = 0; i < map->size; i++) {
        bucket_t *entry = &map->data[i];
        if (entry->next) {
            entry_delete(entry->next);
        }
    }

    /* this frees both the map and the toplevel entries */
    ctu_free(map, sizeof_map(map->size));
}

void *map_get(map_t *map, const char *key) {
    bucket_t *entry = map_bucket(map, key);
    return entry_get(entry, key);
}

void map_set(map_t *map, const char *key, void *value) {
    bucket_t *entry = map_bucket(map, key);

    while (entry != NULL) {
        if (entry->key == NULL) {
            entry->key = key;
            entry->value = value;
            return;
        } else if (streq(entry->key, key)) {
            entry->value = value;
            return;
        } else if (entry->next != NULL) {
            entry = entry->next;
        } else {
            entry->next = bucket_new(key, value);
            return;
        }
    }
}

void map_apply(map_t *map, void *user, map_apply_t func) {
    for (size_t i = 0; i < map->size; i++) {
        bucket_t *entry = &map->data[i];
        while (entry && entry->key) {
            func(user, entry->value);
            entry = entry->next;
        }
    }
}

vector_t *map_collect(map_t *map, map_collect_t filter) {
    vector_t *result = vector_new(map->size);
    
    for (size_t i = 0; i < map->size; i++) {
        bucket_t *entry = &map->data[i];
        while (entry && entry->key) {
            if (filter(entry->value)) {
                vector_push(&result, entry->value);
            }

            entry = entry->next;
        }
    }
    
    return result;
}

vector_t *map_values(map_t *map) {
    vector_t *result = vector_new(map->size);

    for (size_t i = 0; i < map->size; i++) {
        bucket_t *entry = &map->data[i];
        while (entry && entry->key) {
            vector_push(&result, entry->value);
            entry = entry->next;
        }
    }

    return result;
}

// vector internals

static size_t vector_size(size_t size) {
    return sizeof(vector_t) + (size * sizeof(void *));
}

#define VEC (*vector)

static void vector_ensure(vector_t **vector, size_t size) {
    if (size >= VEC->size) {
        size_t old = VEC->size;
        size_t resize = (size + 1) * 2;
        VEC->size = resize;
        VEC = ctu_realloc(VEC, vector_size(old), vector_size(resize));
    }
}

// vector public api

vector_t *vector_new(size_t size) {
    vector_t *vector = ctu_malloc(vector_size(size));
    
    vector->size = size;
    vector->used = 0;

    return vector;
}

vector_t *vector_of(size_t len) {
    vector_t *vector = vector_new(len);
    vector->used = len;
    return vector;
}

vector_t *vector_init(void *value) {
    vector_t *vector = vector_new(1);
    vector_push(&vector, value);
    return vector;
}

void vector_delete(vector_t *vector) {
    ctu_free(vector, vector_size(vector->size));
}

void vector_push(vector_t **vector, void *value) {
    vector_ensure(vector, VEC->used + 1);
    VEC->data[VEC->used++] = value;
}

void *vector_pop(vector_t *vector) {
    return vector->data[--vector->used];
}

void vector_set(vector_t *vector, size_t index, void *value) {
    vector->data[index] = value;
}

void *vector_get(const vector_t *vector, size_t index) {
    return vector->data[index];
}

void *vector_tail(const vector_t *vector) {
    return vector->data[vector->used - 1];
}

void **vector_data(vector_t *vector) {
    return vector->data;
}

size_t vector_len(const vector_t *vector) {
    return vector->used;
}

vector_t *vector_join(const vector_t *lhs, const vector_t *rhs) {
    size_t lhs_len = vector_len(lhs);
    size_t rhs_len = vector_len(rhs);

    size_t len = lhs_len + rhs_len;

    vector_t *out = vector_new(len);
    out->used = len;

    for (size_t i = 0; i < lhs_len; i++) {
        vector_set(out, i, vector_get(lhs, i));
    }

    for (size_t i = 0; i < rhs_len; i++) {
        vector_set(out, lhs_len + i, vector_get(rhs, i));
    }

    return out;
}

vector_t *vector_map(const vector_t *vector, vector_apply_t func) {
    size_t len = vector_len(vector);
    vector_t *out = vector_of(len);

    for (size_t i = 0; i < len; i++) {
        void *value = vector_get(vector, i);
        void *result = func(value);
        vector_set(out, i, result);
    }

    return out;
}
