#include "common.h"

#include "std/map.h"
#include "std/str.h"

#include "base/memory.h"
#include "base/panic.h"

typedef struct fs_virtual_t
{
    const char *name;
} fs_virtual_t;

typedef struct vfs_dir_t 
{
    map_t *children; // map_t<const char *, inode_t *>
} vfs_dir_t;

typedef struct vfs_file_t
{
    io_t *io;
} vfs_file_t;

static inode_t *vfs_new_dir(inode_t *parent)
{
    vfs_dir_t dir = {
        .children = map_new(32)
    };

    return inode_dir(parent, &dir, sizeof(vfs_dir_t));
}

static inode_t *vfs_new_file(inode_t *parent, const char *name)
{
    vfs_file_t file = {
        .io = io_blob(name, 0x1000)
    };

    return inode_file(parent, &file, sizeof(vfs_file_t));
}

static inode_t *vfs_query(inode_t *inode, const char *name)
{
    CTASSERT(inode_is(inode, eNodeDir));

    vfs_dir_t *dir = inode_data(inode);
    return map_get(dir->children, name);
}

static inode_t *vfs_mkdir(inode_t *parent, const char *name)
{
    CTASSERT(inode_is(parent, eNodeDir));

    vfs_dir_t *dir = inode_data(parent);
    inode_t *inode = vfs_new_dir(parent);

    map_set(dir->children, name, inode);

    return inode;
}

static inode_t *vfs_open(inode_t *parent, const char *name, file_flags_t flags)
{
    CTASSERT(inode_is(parent, eNodeDir));

    vfs_dir_t *dir = inode_data(parent);
    inode_t *inode = vfs_new_file(parent, name);

    map_set(dir->children, name, inode);

    return inode;
}

static io_t *vfs_file(inode_t *inode)
{
    CTASSERT(inode_is(inode, eNodeFile));

    vfs_file_t *file = inode_data(inode);
    return file->io;
}

static const fs_callbacks_t kVirtual = {
    .inodeQuery = vfs_query,

    .inodeDir = vfs_mkdir,
    .inodeOpen = vfs_open,

    .inodeFile = vfs_file
};

fs_t *fs_virtual(const char *name)
{
    fs_virtual_t fs = {
        .name = name
    };

    inode_t *root = vfs_new_dir(NULL);

    return fs_new(&kVirtual, root, &fs, sizeof(fs_virtual_t));
}
