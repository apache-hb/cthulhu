// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

#include "arena/arena.h"
#include "os/os.h"

#include "std/str.h"

#include "base/util.h"
#include "base/panic.h"

// inode api

fs_inode_t gInvalidFileNode = {
    .type = eOsNodeNone
};

fs_inode_t *inode_new(os_dirent_t type, const void *data, size_t size, arena_t *arena)
{
    CT_ASSERT_RANGE(type, 0, eOsNodeCount - 1);

    fs_inode_t *inode = ARENA_MALLOC(sizeof(fs_inode_t) + size, "inode", NULL, arena);
    inode->type = type;
    ctu_memcpy(inode->data, data, size);
    return inode;
}

fs_inode_t *inode_file(const void *data, size_t size, arena_t *arena)
{
    return inode_new(eOsNodeFile, data, size, arena);
}

fs_inode_t *inode_dir(const void *data, size_t size, arena_t *arena)
{
    return inode_new(eOsNodeDir, data, size, arena);
}

void *inode_data(fs_inode_t *inode)
{
    CTASSERT(inode != NULL);

    return inode->data;
}

void *iter_data(fs_iter_t *iter)
{
    CTASSERT(iter != NULL);

    return iter->data;
}

bool inode_is(fs_inode_t *inode, os_dirent_t type)
{
    CTASSERT(inode != NULL);
    CTASSERT(type < eOsNodeCount);

    return inode->type == type;
}

// helpers

os_error_t mkdir_recursive(const char *path, arena_t *arena)
{
    CTASSERT(path != NULL);

    size_t index = str_rfind(path, CT_NATIVE_PATH_SEPARATOR);
    if (index != SIZE_MAX)
    {
        // create parent directory
        char *parent = arena_strndup(path, index, arena);
        os_error_t result = mkdir_recursive(parent, arena);
        if (result != 0 && result != eOsExists) { return result; }
    }

    // create this directory
    return os_dir_create(path);
}

// fs api

fs_t *fs_new(fs_inode_t *root, const fs_callbacks_t *cb, const void *data, size_t size, arena_t *arena)
{
    CTASSERT(root != NULL);
    CTASSERT(cb != NULL);

    fs_t *fs = ARENA_MALLOC(sizeof(fs_t) + size, "fs", cb, arena);
    fs->arena = arena;
    fs->cb = cb;
    fs->root = root;

    ctu_memcpy(fs->data, data, size);

    return fs;
}

void *fs_data(fs_t *fs)
{
    CTASSERT(fs != NULL);

    return fs->data;
}
