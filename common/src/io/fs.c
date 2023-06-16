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

static inode_t *open_inode(fs_t *fs, inode_t *parent, const char *name, file_flags_t flags)
{
    return fs->cb->inodeOpen(parent, name, flags);
}

static io_t *inode_io(fs_t *fs, inode_t *parent)
{
    return fs->cb->inodeFile(parent);
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

static inode_t *checked_open(fs_t *fs, inode_t *parent, const char *name, file_flags_t flags)
{
    inode_t *node = query_inode(fs, parent, name);
    if (inode_is(node, eNodeFile))
    {
        return node;
    }

    if (node != NULL)
    {
        return NULL;
    }

    return open_inode(fs, parent, name, flags);
}

static inode_t *recursive_mkdir(fs_t *fs, inode_t *parent, vector_t *parts)
{
    size_t len = vector_len(parts);
    inode_t *node = fs->root;

    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *next = checked_mkdir(fs, node, part);
        
        if (next == NULL)
        {
            return NULL; // TODO: error
        }

        node = next;
    }

    return node;
}

void fs_mkdir(fs_t *fs, const char *path)
{
    vector_t *parts = str_split(path, "/");
    recursive_mkdir(fs, fs->root, parts);
}

io_t *fs_open(fs_t *fs, const char *path, file_flags_t flags)
{
    char *dir = str_path(path);

    inode_t *inode = recursive_mkdir(fs, fs->root, str_split(dir, "/"));

    char *name = str_filename(path);

    inode_t *file = checked_open(fs, inode, name, flags);

    return inode_io(fs, file);
}
