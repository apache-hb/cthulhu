#include "cthulhu/util/vector.h"
#include "cthulhu/util/util.h"

#include <stdint.h>

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

size_t vector_find(vector_t *vector, const void *element) {
    for (size_t i = 0; i < vector_len(vector); i++) {
        if (vector_get(vector, i) == element) {
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
