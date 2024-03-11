// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_fs_api.h>

#include "os/core.h"

CT_BEGIN_API

/// @ingroup fs
/// @{

/// @brief a filesystem interface
typedef struct fs_t fs_t;

/// @brief a filesystem inode
typedef struct fs_inode_t
{
    /// @brief the type of the inode
    os_dirent_t type;

    /// @brief internal data
    char data[];
} fs_inode_t;

/// @brief a filesystem iterator
typedef struct fs_iter_t
{
    /// @brief the parent filesystem
    fs_t *fs;

    /// @brief internal data
    char data[];
} fs_iter_t;

/// @}

CT_END_API
