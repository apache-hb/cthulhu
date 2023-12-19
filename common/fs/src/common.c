#include "common.h"

#include "std/str.h"

#include "memory/memory.h"
#include "base/panic.h"

#include <string.h>

// inode api

inode_t gInvalidINode = {
    .type = eNodeInvalid
};

static inode_t *inode_new(inode_type_t type, const void *data, size_t size)
{
    CTASSERT(type < eNodeTotal);

    inode_t *inode = MEM_ALLOC(sizeof(inode_t) + size, "inode", NULL);
    inode->type = type;
    memcpy(inode->data, data, size);
    return inode;
}

inode_t *inode_file(const void *data, size_t size)
{
    return inode_new(eNodeFile, data, size);
}

inode_t *inode_dir(const void *data, size_t size)
{
    return inode_new(eNodeDir, data, size);
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

OS_RESULT(bool) mkdir_recursive(const char *path)
{
    size_t index = str_rfind(path, NATIVE_PATH_SEPARATOR);
    if (index != SIZE_MAX)
    {
        // create parent directory
        char *parent = ctu_strndup(path, index);
        OS_RESULT(bool) result = mkdir_recursive(parent);
        if (os_error(result) != 0) { return result; }
    }

    // create this directory
    return os_dir_create(path);
}

// fs api

void fs_delete(fs_t *fs)
{
    CTASSERT(fs != NULL);

    if (fs->cb->pfn_delete != NULL)
    {
        fs->cb->pfn_delete(fs);
    }

    ctu_free(fs);
}

fs_t *fs_new(inode_t *root, const fs_callbacks_t *cb, const void *data, size_t size)
{
    CTASSERT(cb != NULL);

    fs_t *fs = MEM_ALLOC(sizeof(fs_t) + size, "fs", cb);
    fs->cb = cb;
    fs->root = root;

    memcpy(fs->data, data, size);

    return fs;
}

void *fs_data(fs_t *fs)
{
    CTASSERT(fs != NULL);

    return fs->data;
}
