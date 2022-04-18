#pragma once

#include "macros.h"
#include <stddef.h>

/**
 * @brief a node in a chain of set entries
 */
typedef struct item_t {
    const char *key; ///< the key to this bucket
    struct item_t *next; ///< the next bucket in the chain
} item_t;

/**
 * @brief a hashset of strings
 */
typedef struct {
    size_t size; ///< the number of buckets
    item_t items[]; ///< the buckets
} set_t;

/**
 * @brief create a new set with a given number of buckets
 * 
 * @param size 
 * @return set_t* 
 */
set_t *set_new(size_t size);
void set_delete(set_t *set);

const char* set_add(set_t *set, const char *key);
bool set_contains(set_t *set, const char *key);
