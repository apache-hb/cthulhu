#include "util.h"
#include "io.h"

#include "ctu/util/report.h"

#include <string.h>
#include <stdlib.h>
#include <gmp.h>

#ifndef _WIN32
#   include <sys/mman.h>
#endif

void *ctu_malloc(size_t size) {
    return malloc(size);
}

void *ctu_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void ctu_free(void *ptr) {
    free(ptr);
}

static void *ctu_gmp_realloc(void *ptr, size_t old_size, size_t new_size) {
    UNUSED(old_size);
    return ctu_realloc(ptr, new_size);
}

static void ctu_gmp_free(void *ptr, size_t size) {
    UNUSED(size);
    DELETE(ptr);
}

void init_memory(void) {
    mp_set_memory_functions(
        ctu_malloc, 
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

static size_t string_hash(const char *str) {
    size_t hash = 0;

    while (*str) {
        hash = (hash << 5) - hash + *str++;
    }

    return hash;
}

file_t *ctu_open(const char *path, const char *mode) {
    FILE *fp = fopen(path, mode);

    if (fp == NULL) {
        return NULL;
    }

    file_t *file = NEW(file_t);
    file->file = fp;
    file->path = path;

    return file;
}

void ctu_close(file_t *fp) {
    if (fp->file) {
        fclose(fp->file);
    }

    DELETE(fp);
}

bool ctu_valid(const file_t *fp) {
    return fp->file != NULL;
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
    return sizeof(map_t) + (size * sizeof(entry_t));
}

static entry_t *entry_new(const char *key, void *value) { 
    entry_t *entry = NEW(entry_t);
    entry->key = key;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

static void *entry_get(const entry_t *entry, const char *key) {
    if (entry->key && strcmp(entry->key, key) == 0) {
        return entry->value;
    }

    if (entry->next) {
        return entry_get(entry->next, key);
    }

    return NULL;
}

static void entry_delete(entry_t *entry) {
    if (entry->next) {
        entry_delete(entry->next);
    }

    DELETE(entry);
}

/* find which bucket a key should be in */
static entry_t *map_bucket(map_t *map, const char *key) {
    size_t hash = string_hash(key);
    size_t index = hash % map->size;
    entry_t *entry = &map->data[index];
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
        entry_t *entry = &map->data[i];
        if (entry->next) {
            entry_delete(entry->next);
        }
    }

    /* this frees both the map and the toplevel entries */
    DELETE(map);
}

void *map_get(map_t *map, const char *key) {
    entry_t *entry = map_bucket(map, key);
    return entry_get(entry, key);
}

void map_set(map_t *map, const char *key, void *value) {
    entry_t *entry = map_bucket(map, key);

    while (entry) {
        if (entry->key == NULL) {
            entry->key = key;
            entry->value = value;
            return;
        } else if (strcmp(entry->key, key) == 0) {
            entry->value = value;
            return;
        } else if (entry->next != NULL) {
            entry = entry->next;
        } else {
            entry->next = entry_new(key, value);
            return;
        }
    }
}

void map_apply(map_t *map, void *user, map_apply_t func) {
    for (size_t i = 0; i < map->size; i++) {
        entry_t *entry = &map->data[i];
        while (entry && entry->key) {
            func(user, entry->value);
            entry = entry->next;
        }
    }
}

vector_t *map_collect(map_t *map, map_collect_t filter) {
    vector_t *result = vector_new(map->size);
    
    for (size_t i = 0; i < map->size; i++) {
        entry_t *entry = &map->data[i];
        while (entry && entry->key) {
            if (filter(entry->value)) {
                vector_push(&result, entry->value);
            }

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
        size_t resize = (size + 1) * 2;
        VEC->size = resize;
        VEC = ctu_realloc(VEC, vector_size(resize));
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
    DELETE(vector);
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
