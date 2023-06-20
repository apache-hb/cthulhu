#include "common.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

// inode api

inode2_t kInvalidINode = {
    .type = eNodeInvalid
};

static inode2_t *inode_new(inode2_type_t type, const void *data, size_t size)
{
    CTASSERT(type < eNodeTotal);

    inode2_t *inode = ctu_malloc(sizeof(inode2_t) + size);
    inode->type = type;
    memcpy(inode->data, data, size);
    return inode;
}

inode2_t *inode2_file(const void *data, size_t size)
{
    return inode_new(eNodeFile, data, size);
}

inode2_t *inode2_dir(const void *data, size_t size)
{
    return inode_new(eNodeDir, data, size);
}

void *inode2_data(inode2_t *inode)
{
    CTASSERT(inode != NULL);
    
    return inode->data;
}

bool inode2_is(inode2_t *inode, inode2_type_t type)
{
    CTASSERT(inode != NULL);
    CTASSERT(type < eNodeTotal);

    return inode->type == type;
}

// fs api

fs2_t *fs2_new(reports_t *reports, inode2_t *root, const fs2_interface_t *cb, const void *data, size_t size)
{
    CTASSERT(reports != NULL);
    CTASSERT(cb != NULL);

    fs2_t *fs = ctu_malloc(sizeof(fs2_t) + size);
    fs->reports = reports;
    fs->cb = cb;
    fs->root = root;

    memcpy(fs->data, data, size);
    
    return fs;
}

void *fs2_data(fs2_t *fs)
{
    CTASSERT(fs != NULL);
    return fs->data;
}
