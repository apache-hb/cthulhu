#include "common.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

// inode api

inode_t kInvalidINode = {
    .type = eNodeInvalid
};

static inode_t *inode_new(inode_type_t type, const void *data, size_t size)
{
    CTASSERT(type < eNodeTotal);

    inode_t *inode = ctu_malloc(sizeof(inode_t) + size);
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

// fs api

fs_t *fs_new(reports_t *reports, inode_t *root, const fs_interface_t *cb, const void *data, size_t size)
{
    CTASSERT(reports != NULL);
    CTASSERT(cb != NULL);

    fs_t *fs = ctu_malloc(sizeof(fs_t) + size);
    fs->reports = reports;
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