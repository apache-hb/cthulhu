#pragma once

#include "base/analyze.h"
#include <stddef.h>

BEGIN_API

typedef struct typevec_t typevec_t;

typevec_t *typevec_new(size_t size, size_t len);
typevec_t *typevec_of(size_t size, size_t len);
typevec_t *typevec_init(size_t size, const void *value);

size_t typevec_len(typevec_t *vec);

void typevec_set(typevec_t *vec, size_t index, const void *src);
void typevec_get(typevec_t *vec, size_t index, void *dst);
void typevec_tail(typevec_t *vec, void *dst);

void typevec_push(typevec_t *vec, const void *src);
void typevec_pop(typevec_t *vec, void *dst);

/**
 * @brief get a pointer to the value at the given index
 *
 * @note the pointer is only valid until the next call to typevec_push or typevec_pop
 *
 * @param vec the vector to get the value from
 * @param index the index to get the value from
 * @return void* a pointer to the value
 */
void *typevec_offset(typevec_t *vec, size_t index);

END_API
