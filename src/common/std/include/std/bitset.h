#pragma once

#include <ctu_std_api.h>

#include "core/analyze.h"

#include <stdbool.h>
#include <stdint.h>

CT_BEGIN_API

typedef struct arena_t arena_t;

/// @defgroup bitset Simple bitset
/// @ingroup standard
/// @brief Simple bitset
/// @{

/// @brief a bitset
/// @warning this is an internal type and should not be used directly
typedef struct bitset_t
{
    /// @brief the number of bytes in @a data
    FIELD_RANGE(>, 0) size_t words;

    /// @brief the data for the bitset
    FIELD_SIZE(words) void *data;
} bitset_t;

#define CT_BITSET_ARRAY(arr) { sizeof(arr), arr }

/// @brief create a new bitset
/// @pre @p bits > 0
/// @pre @p arena is not NULL
///
/// @param bits the number of bits in the bitset
/// @param arena the arena to allocate from
///
/// @return the new bitset
CT_NODISCARD
CT_STD_API bitset_t bitset_new(IN_RANGE(>, 0) size_t bits, IN_NOTNULL arena_t *arena);

/// @brief create a bitset from a buffer
/// @pre @p data is not NULL
/// @pre @p words > 0
///
/// @param data the buffer to use for the bitset
/// @param words the number of bytes in the buffer
///
/// @return the new bitset
CT_NODISCARD
CT_STD_API bitset_t bitset_of(IN_READS(words) void *data, size_t words);

/// @brief test if any bits in a given mask are set
///
/// @param set the bitset to test
/// @param mask the mask to test
///
/// @return true if any bits in the mask are set
CT_STD_API bool bitset_any(IN_NOTNULL const bitset_t set, IN_NOTNULL const bitset_t mask);

/// @brief test if all bits in a given mask are set
///
/// @param set the bitset to test
/// @param mask the mask to test
///
/// @return true if all bits in the mask are set
CT_STD_API bool bitset_all(IN_NOTNULL const bitset_t set, IN_NOTNULL const bitset_t mask);

/// @brief test if a bit is set
///
/// @param set the bitset to test
/// @param index the index of the bit to test
///
/// @return true if the bit is set
CT_STD_API bool bitset_test(IN_NOTNULL const bitset_t set, size_t index);

/// @brief set a bit
///
/// @param set the bitset to modify
/// @param index the index of the bit to set
CT_STD_API void bitset_set(IN_NOTNULL bitset_t set, size_t index);

/// @brief clear a bit
///
/// @param set the bitset to modify
/// @param index the index of the bit to clear
CT_STD_API void bitset_clear(IN_NOTNULL bitset_t set, size_t index);

/// @brief reset all bits in a bitset
///
/// @param set the bitset to reset
CT_STD_API void bitset_reset(IN_NOTNULL bitset_t set);

/// @brief get the number of bits in a bitset
///
/// @param set the bitset to get the length of
///
/// @return the number of bits in the bitset
CT_STD_API size_t bitset_len(IN_NOTNULL const bitset_t set);

/// @}

CT_END_API
