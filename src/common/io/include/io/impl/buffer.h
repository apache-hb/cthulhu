// SPDX-License-Identifier: LGPL-3.0-only
#pragma once

// for sizeof(io_t)
#include "io/impl.h" // IWYU pragma: export

CT_BEGIN_API

/// @ingroup io_impl
/// @{

/// @brief a read/write in memory file
/// @warning this is an internal structure and should not be used directly
typedef struct io_buffer_impl_t
{
    /// @brief stored data
    char *data;

    /// @brief used data
    size_t used;

    /// @brief total size of data
    size_t total;

    /// @brief current offset in data
    size_t offset;
} io_buffer_impl_t;

#define IO_BUFFER_SIZE (sizeof(io_buffer_impl_t) + sizeof(io_t))

/// @}

CT_END_API
