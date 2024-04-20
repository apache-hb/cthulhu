// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

// for sizeof(io_t)
#include "io/impl.h" // IWYU pragma: export

#include "os/os.h"

CT_BEGIN_API

/// @ingroup io_impl
/// @{

/// @brief a file descriptor
/// @warning this is an internal structure and should not be used directly
typedef struct io_file_impl_t
{
    /// @brief native file descriptor
    os_file_t file;

    /// @brief memory mapping
    os_mapping_t mapping;
} io_file_impl_t;

#define IO_FILE_SIZE (sizeof(io_file_impl_t) + sizeof(io_t))

/// @}

CT_END_API
