#include "common.h"

#include "base/memory.h"

#include <string.h>

// inode internals

static inode_t *inode_new(inode_type_t type, inode_t *parent, void *data, size_t size)
{
    inode_t *inode = ctu_malloc(sizeof(inode_t) + size);

    inode->type = type;
    inode->parent = parent;
    
    memcpy(inode->data, data, size);

    return inode;
}

inode_t *inode_dir(inode_t *parent, void *data, size_t size)
{
    return inode_new(eNodeDir, parent, data, size);
}

inode_t *inode_file(inode_t *parent, void *data, size_t size)
{
    return inode_new(eNodeFile, parent, data, size);
}


void *inode_data(inode_t *inode)
{
    return inode->data;
}

bool inode_is(inode_t *inode, inode_type_t type)
{
    return inode != NULL && inode->type == type;
}

// fs internals

fs_t *fs_new(const fs_callbacks_t *cb, inode_t *root, void *data, size_t size)
{
    fs_t *fs = ctu_malloc(sizeof(fs_t) + size);
    
    fs->cb = cb;
    fs->root = root;

    memcpy(fs->data, data, size);

    return fs;
}

void *fs_data(fs_t *fs)
{
    return fs->data;
}
