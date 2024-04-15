// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_base_api.h>

#include "core/analyze.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

CT_BEGIN_API

/// @defgroup bitset Simple bitset
/// @ingroup base
/// @brief Simple bitset
/// @{

/// @brief a bitset
/// @warning this is an internal type and should not be used directly
typedef struct bitset_t
{
    /// @brief the number of bytes in @a data
    FIELD_RANGE(0, SIZE_MAX) size_t words;

    /// @brief the data for the bitset
    FIELD_SIZE(words) void *data;
} bitset_t;

#define CT_BITSET_ARRAY(arr) { sizeof(arr), arr }

/// @brief create a bitset from a buffer
/// @pre @p data is not NULL
/// @pre @p words > 0
///
/// @param data the buffer to use for the bitset
/// @param words the number of bytes in the buffer
///
/// @return the new bitset
CT_NODISCARD
CT_BASE_API bitset_t bitset_of(IN_READS(words) void *data, size_t words);

/// @brief scan for the next free bit and set it
///
/// @param set the bitset to scan
/// @param start the index to start scanning from
///
/// @return the index of the next free bit, or SIZE_MAX if none are free
CT_NODISCARD
CT_BASE_API size_t bitset_set_first(IN_NOTNULL bitset_t set, size_t start);

/// @brief test if any bits in a given mask are set
///
/// @param set the bitset to test
/// @param mask the mask to test
///
/// @return true if any bits in the mask are set
CT_BASE_API bool bitset_any(IN_NOTNULL const bitset_t set, IN_NOTNULL const bitset_t mask);

/// @brief test if all bits in a given mask are set
///
/// @param set the bitset to test
/// @param mask the mask to test
///
/// @return true if all bits in the mask are set
CT_BASE_API bool bitset_all(IN_NOTNULL const bitset_t set, IN_NOTNULL const bitset_t mask);

/// @brief test if a bit is set
///
/// @param set the bitset to test
/// @param index the index of the bit to test
///
/// @return true if the bit is set
CT_BASE_API bool bitset_test(IN_NOTNULL const bitset_t set, size_t index);

/// @brief set a bit
///
/// @param set the bitset to modify
/// @param index the index of the bit to set
CT_BASE_API void bitset_set(IN_NOTNULL bitset_t set, size_t index);

/// @brief clear a bit
///
/// @param set the bitset to modify
/// @param index the index of the bit to clear
CT_BASE_API void bitset_clear(IN_NOTNULL bitset_t set, size_t index);

/// @brief reset all bits in a bitset
///
/// @param set the bitset to reset
CT_BASE_API void bitset_reset(IN_NOTNULL bitset_t set);

/// @brief get the number of bits in a bitset
///
/// @param set the bitset to get the length of
///
/// @return the number of bits in the bitset
CT_BASE_API size_t bitset_len(IN_NOTNULL const bitset_t set);

/// @}

CT_END_API
