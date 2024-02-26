// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_fs_api.h>

#include "os/core.h"

CT_BEGIN_API

typedef struct fs_inode_t
{
    os_dirent_t type;

    // TODO: cant use alignas here because of an msvc bug
    char data[];
} fs_inode_t;

CT_END_API
