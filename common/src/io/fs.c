#include "fs/common.h"

#include "base/memory.h"
#include "base/panic.h"

#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"

#include <string.h>

static inode_t *query_inode(fs_t *fs, inode_t *node, const char *name)
{
    return fs->cb->inodeQuery(node, name);
}

static inode_t *mkdir_inode(fs_t *fs, inode_t *parent, const char *name)
{
    return fs->cb->inodeDir(parent, name);
}

static inode_t *checked_mkdir(fs_t *fs, inode_t *parent, const char *name)
{
    inode_t *node = query_inode(fs, parent, name);
    if (inode_is(node, eNodeDir))
    {
        return node;
    }

    if (node != NULL)
    {
        return NULL;
    }

    return mkdir_inode(fs, parent, name);
}

void fs_mkdir(fs_t *fs, const char *path)
{
    vector_t *parts = str_split(path, "/");
    size_t len = vector_len(parts);
    inode_t *node = fs->root;

    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *next = checked_mkdir(fs, node, part);
        
        if (next == NULL)
        {
            // TODO: error
            return;
        }

        node = next;
    }
}