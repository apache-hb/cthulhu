#include "common.h"

#include "arena/arena.h"
#include "os/os.h"

#include "std/str.h"

#include "base/util.h"
#include "base/panic.h"

// inode api

inode_t gInvalidFileNode = {
    .type = eNodeInvalid
};

static inode_t *inode_new(inode_type_t type, const void *data, size_t size, arena_t *arena)
{
    CTASSERT(type < eNodeTotal);

    inode_t *inode = ARENA_MALLOC(sizeof(inode_t) + size, "inode", NULL, arena);
    inode->type = type;
    ctu_memcpy(inode->data, data, size);
    return inode;
}

inode_t *inode_file(const void *data, size_t size, arena_t *arena)
{
    return inode_new(eNodeFile, data, size, arena);
}

inode_t *inode_dir(const void *data, size_t size, arena_t *arena)
{
    return inode_new(eNodeDir, data, size, arena);
}

void *inode_data(inode_t *inode)
{
    CTASSERT(inode != NULL);

    return inode->data;
}

bool inode_is(inode_t *inode, inode_type_t type)
{
    CTASSERT(inode != NULL);
    CTASSERT(type < eNodeTotal);

    return inode->type == type;
}

// helpers

os_error_t mkdir_recursive(const char *path, bool *create, arena_t *arena)
{
    CTASSERT(path != NULL);
    CTASSERT(create != NULL);

    size_t index = str_rfind(path, CT_NATIVE_PATH_SEPARATOR);
    if (index != SIZE_MAX)
    {
        // create parent directory
        char *parent = arena_strndup(path, index, arena);
        os_error_t result = mkdir_recursive(parent, create, arena);
        if (result != 0) { return result; }
    }

    // create this directory
    return os_dir_create(path, create);
}

// fs api

fs_t *fs_new(inode_t *root, const fs_callbacks_t *cb, const void *data, size_t size, arena_t *arena)
{
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
