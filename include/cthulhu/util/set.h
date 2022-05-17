#pragma once

#include "macros.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief a node in a chain of set entries
 */
typedef struct item_t
{
    const void *key;     ///< the key to this bucket
    struct item_t *next; ///< the next bucket in the chain
} item_t;

/**
 * @brief a hashset of strings
 */
typedef struct
{
    size_t size;                     ///< the number of buckets
    FIELD_SIZE(size) item_t items[]; ///< the buckets
} set_t;

/**
 * @brief create a new set with a given number of buckets
 *
 * @param size the number of buckets
 * @return the created set
 */
NODISCARD
set_t *set_new(size_t size);

/**
 * @brief delete a set
 *
 * @param set the set to delete
 */
void set_delete(set_t *set);

/**
 * @brief add a string to a set
 *
 * @param set the set to add to
 * @param key the key to add
 * @return a pointer to the deduplicated key
 */
const char *set_add(set_t *set, const char *key);

/**
 * @brief check if a set contains a key
 *
 * @param set the set to check
 * @param key the key to check
 * @return true if the set contains the key
 */
NODISCARD
bool set_contains(set_t *set, const char *key);

const void *set_add_ptr(set_t *set, const void *key);

NODISCARD
bool set_contains_ptr(set_t *set, const void *key);

void set_reset(set_t *set);
