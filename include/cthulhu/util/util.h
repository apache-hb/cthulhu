#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "sizes.h"
#include "macros.h"

/**
 * memory managment functions
 * 
 * TODO: arenas will be faster
 */
void ctu_free(void *ptr) NONULL;
void *ctu_malloc(size_t size) ALLOC(ctu_free);
void *ctu_realloc(void *ptr, size_t size) NOTNULL(1) ALLOC(ctu_free);
char *ctu_strdup(const char *str) NONULL ALLOC(ctu_free);
char *ctu_strndup(const char *str, size_t len) NONULL ALLOC(ctu_free);
char *ctu_strdup_len(const char *str, size_t *len) NONULL ALLOC(ctu_free);
void *ctu_memdup(const void *ptr, size_t size) NOTNULL(1) ALLOC(ctu_free);

#if ENABLE_TUNING
typedef struct {
    size_t mallocs; // calls to malloc
    size_t reallocs; // calls to realloc
    size_t frees; // calls to free

    size_t current; // current memory allocated
    size_t peak; // peak memory allocated
} counters_t;
counters_t get_counters(void);
counters_t reset_counters(void);
#endif

/**
 * @brief init gmp with our own allocation functions
 */
void init_gmp(void);

/**
 * @brief box a value onto the heap from the stack
 * 
 * @param ptr the value to box
 * @param size the size of the value
 * 
 * @return the boxed value
 * 
 * @see BOX should be used to use this
 */
void *ctu_box(const void *ptr, size_t size) NOTNULL(1) ALLOC(ctu_free);
#define BOX(name) ctu_box(&name, sizeof(name))

/**
 * @def BOX(name) box a value onto the heap from the stack
 */
