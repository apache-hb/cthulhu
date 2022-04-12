#define _POSIX_SOURCE

#include "cthulhu/util/util.h"
#include "cthulhu/util/io.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/str.h"
#include "cthulhu/util/macros.h"

#include "compat.h"

#include <string.h>
#include <stdlib.h>
#include <gmp.h>

#if ENABLE_TUNING
#   include <malloc.h>

static counters_t counters = { 
    .mallocs = 0, 
    .reallocs = 0,
    .frees = 0,

    .current = 0,
    .peak = 0
};

counters_t get_counters(void) {
    return counters;
}

counters_t reset_counters(void) {
    counters_t out = counters;

    counters.mallocs = 0;
    counters.reallocs = 0;
    counters.frees = 0;
    counters.current = 0;
    counters.peak = 0;

    return out;
}

static void adjust_memory_stats(ssize_t size) {
    counters.current += size;
    if (counters.current > counters.peak) {
        counters.peak = counters.current;
    }
}

static void *tuning_malloc(size_t size) {
    counters.mallocs++;
    void *result = malloc(size);

    adjust_memory_stats(malloc_usable_size(result));

    return result;
}

static void *tuning_realloc(void *ptr, size_t size) {
    counters.reallocs++;

    adjust_memory_stats(-malloc_usable_size(ptr));

    void *result = realloc(ptr, size);

    adjust_memory_stats(malloc_usable_size(result));

    return result;
}

static void tuning_free(void *ptr) {
    counters.frees++;

    adjust_memory_stats(-malloc_usable_size(ptr));

    free(ptr);
}

#   define MALLOC(size) tuning_malloc(size)
#   define REALLOC(ptr, size) tuning_realloc(ptr, size)
#   define FREE(ptr) tuning_free(ptr)

#else
#   define MALLOC(size) malloc(size)
#   define REALLOC(ptr, size) realloc(ptr, size)
#   define FREE(ptr) free(ptr)
#endif

void *ctu_malloc(size_t size) {
    void *ptr = MALLOC(size);
    CTASSERTF(ptr != NULL, "ctu-malloc of %zu bytes failed", size);
    return ptr;
}

void *ctu_realloc(void *ptr, size_t size) {
    void *data = REALLOC(ptr, size);
    CTASSERT(data != NULL, "ctu-realloc failed");
    return data;
}

void ctu_free(void *ptr) {
    FREE(ptr);
}

void *ctu_box(const void *ptr, size_t size) {
    void *box = ctu_malloc(size);
    memcpy(box, ptr, size);
    return box;
}

static void *ctu_gmp_malloc(size_t size) {
    return ctu_malloc(size);
}

static void *ctu_gmp_realloc(void *ptr, size_t old, size_t size) {
    UNUSED(old);
    return ctu_realloc(ptr, size);
}

static void ctu_gmp_free(void *ptr, size_t size) {
    UNUSED(size);
    ctu_free(ptr);
}

void init_gmp(void) {
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

char *ctu_strndup(const char *str, size_t len) {
    char *out = ctu_malloc(len + 1);
    memcpy(out, str, len);
    out[len] = '\0';
    return out;
}

char *ctu_strdup_len(const char *str, size_t *len) {
    size_t bytes = strlen(str) + 1;
    char *out = ctu_malloc(bytes);
    memcpy(out, str, bytes);
    *len = bytes;
    return out;
}

void *ctu_memdup(const void *ptr, size_t size) {
    void *out = ctu_malloc(size);
    memcpy(out, ptr, size);
    return out;
}

char *ctu_pathjoin(const char *path, const char *file) {
    return format("%s" PATH_SEP "%s", path, file);
}

file_t ctu_fopen(const char *path, const char *mode) {
    logverbose("opening: %s", path);
    file_t file = {
        .path = ctu_realpath(path),
        .file = compat_fopen(path, mode)
    };
    return file;
}

void ctu_close(file_t *fp) {
    if (fp->file) {
        fclose(fp->file);
    }
}

bool file_valid(file_t *fp) {
    return fp->file != NULL;
}

size_t ctu_read(void *dst, size_t total, file_t *fp) {
    return fread(dst, 1, total, fp->file);
}

size_t file_size(FILE *fd) {
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
    if (text != MAP_FAILED) {
        return text;
    }
#endif

    text = ctu_malloc(size + 1);
    fread(text, size, 1, fp->file);
    text[size] = '\0';

    return text;
}

void ctpanic(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    exit(EXIT_INTERAL);
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

HOT static void *entry_get(const bucket_t *entry, const char *key, void *other) {
    if (entry->key && streq(entry->key, key)) {
        return entry->value;
    }

    if (entry->next) {
        return entry_get(entry->next, key, other);
    }

    return other;
}

static size_t ptrhash(uintptr_t key) {
    key = (~key) + (key << 18);
    key ^= key >> 31;
    key *= 21;
    key ^= key >> 11;
    key += key << 6;
    key ^= key >> 22;
    return 0xFFFFFFFF & key;
}

static bucket_t *get_bucket(map_t *map, size_t hash) {
    size_t index = hash % map->size;
    bucket_t *entry = &map->data[index];
    return entry;
}



/* find which bucket a key should be in */
HOT static bucket_t *map_bucket(map_t *map, const char *key) {
    size_t hash = strhash(key);
    return get_bucket(map, hash);
}

HOT static bucket_t *map_bucket_ptr(map_t *map, const void *key) {
    size_t hash = ptrhash((uintptr_t)key);
    return get_bucket(map, hash);
}

HOT static void *entry_get_ptr(const bucket_t *entry, const void *key, void *other) {
    if (entry->key == key) {
        return entry->value;
    }

    if (entry->next) {
        return entry_get_ptr(entry->next, key, other);
    }

    return other;
}

void map_set_ptr(map_t *map, const void *key, void *value) {
    bucket_t *entry = map_bucket_ptr(map, key);

    while (entry) {
        if (entry->key == NULL) {
            entry->key = key;
            entry->value = value;
            return;
        } 
        
        if (entry->key == key) {
            entry->value = value;
            return;
        } 
        
        if (entry->next == NULL) {
            entry->next = bucket_new(key, value);
            return;
        }

        entry = entry->next;
    }
}

void *map_get_ptr(map_t *map, const void *key) {
    return map_get_ptr_default(map, key, NULL);
}

void *map_get_ptr_default(map_t *map, const void *key, void *other) {
    bucket_t *bucket = map_bucket_ptr(map, key);
    return entry_get_ptr(bucket, key, other);
}

void map_copy_by_string(map_t *dst, map_t *src) {
    for (size_t i = 0; i < src->size; i++) {
        bucket_t *entry = &src->data[i];
        while (entry && entry->key) {
            map_set(dst, entry->key, entry->value);
            entry = entry->next;
        }
    }
}

static void clear_keys(bucket_t *buckets, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buckets[i].key = NULL;
        buckets[i].next = NULL;
    }
}

// map public api

map_t *map_new(map_size_t size) {
    ASSUME(size > 0);

    map_t *map = ctu_malloc(sizeof_map(size));

    map->size = size;

    clear_keys(map->data, size);

    return map;
}

void *map_get_default(map_t *map, const char *key, void *other) {
    bucket_t *bucket = map_bucket(map, key);
    return entry_get(bucket, key, other);
}

void *map_get(map_t *map, const char *key) {
    return map_get_default(map, key, NULL);
}

void map_set(map_t *map, const char *key, void *value) {
    bucket_t *entry = map_bucket(map, key);

    while (true) {
        if (entry->key == NULL) {
            entry->key = key;
            entry->value = value;
            break;
        }  
        
        if (streq(entry->key, key)) {
            entry->value = value;
            break;
        } 
        
        if (entry->next == NULL) {
            entry->next = bucket_new(key, value);
            break;
            entry = entry->next;
        } 

        entry = entry->next;
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
    ctu_free(vector);
}

void vector_push(vector_t **vector, void *value) {
    vector_ensure(vector, VEC->used + 1);
    VEC->data[VEC->used++] = value;
}

void vector_drop(vector_t **vector) {
    VEC->used -= 1;
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

void *vector_head(const vector_t *vector) {
    return vector->data[0];
}

void **vector_data(vector_t *vector) {
    return vector->data;
}

size_t vector_len(const vector_t *vector) {
    return vector->used;
}

static bool default_cmp(const void *lhs, const void *rhs) {
    return lhs == rhs;
}

size_t vector_find(vector_t *vector, const void *element, vector_cmp_t cmp) {
    if (cmp == NULL) { cmp = default_cmp; }

    for (size_t i = 0; i < vector_len(vector); i++) {
        if (cmp(vector_get(vector, i), element)) {
            return i;
        }
    }

    return SIZE_MAX;
}

vector_t *vector_slice(vector_t *vector, size_t start, size_t end) {
    vector_t *result = vector_of(end - start);
    for (size_t i = start; i < end; i++) {
        vector_set(result, i - start, vector_get(vector, i));
    }
    return result;
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

vector_t *vector_collect(vector_t *vectors) {
    size_t vecs = vector_len(vectors);
    size_t len = 0;
    for (size_t i = 0; i < vecs; i++) {
        vector_t *vec = vector_get(vectors, i);
        len += vector_len(vec);
    }

    vector_t *out = vector_of(len);
    size_t idx = 0;
    for (size_t i = 0; i < vecs; i++) {
        vector_t *vec = vector_get(vectors, i);
        size_t sub = vector_len(vec);
        for (size_t j = 0; j < sub; j++) {
            vector_set(out, idx++, vector_get(vec, j));
        }
    }

    return out;
}

void vector_reset(vector_t *vec) {
    vec->used = 0;
}
