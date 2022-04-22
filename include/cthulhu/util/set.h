#pragma once

#include "macros.h"
#include <stddef.h>
#include <stdbool.h>

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
 * @param size the number of buckets
 * @return the created set
 */
NODISCARD RESULT_VALID
set_t *set_new(IN_RANGE(>, 0) size_t size);

/**
 * @brief delete a set
 * 
 * @param set the set to delete
 */
void set_delete(IN_NOTNULL set_t *set);

/**
 * @brief add a string to a set
 * 
 * @param set the set to add to
 * @param key the key to add
 * @return a pointer to the deduplicated key
 */
RESULT_NOTNULL_STRING
const char* set_add(
    IN_NOTNULL set_t *set, 
    IN_NOTNULL_STRING const char *key
);

/**
 * @brief check if a set contains a key
 * 
 * @param set the set to check
 * @param key the key to check
 * @return true if the set contains the key
 */
NODISCARD
bool set_contains(
    IN_NOTNULL set_t *set, 
    IN_NOTNULL_STRING const char *key
);
