// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

// for sizeof(io_t)
#include "io/impl.h" // IWYU pragma: export

CT_BEGIN_API

/// @ingroup io_impl
/// @{

/// @brief a non-owning, readonly view of a buffer
/// @warning this is an internal structure and should not be used directly
typedef struct io_view_impl_t
{
    /// @brief pointer to data
    const char *data;

    /// @brief size of data
    size_t size;

    /// @brief current offset in data
    size_t offset;
} io_view_impl_t;

#define IO_VIEW_SIZE (sizeof(io_view_impl_t) + sizeof(io_t))

/// @}

CT_END_API
