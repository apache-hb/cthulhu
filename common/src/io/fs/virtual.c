#include "common.h"
#include "virtual.h"

#include "std/map.h"
#include "std/str.h"

#include "base/memory.h"
#include "base/panic.h"

typedef struct fs_virtual_t
{
    const char *name; ///< name of the virtual filesystem
} fs_virtual_t;

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

static inode_t *vfs_query(fs_t *fs, inode_t *inode, const char *name)
{
    UNUSED(fs);
    CTASSERT(inode_is(inode, eNodeDir));

    vfs_dir_t *dir = inode_data(inode);
    return map_get(dir->children, name);
}

static inode_t *vfs_create_dir(fs_t *fs, inode_t *parent, const char *name)
{
    UNUSED(fs);
    CTASSERT(inode_is(parent, eNodeDir));

    vfs_dir_t *dir = inode_data(parent);
    inode_t *inode = vfs_new_dir(parent);

    map_set(dir->children, name, inode);

    return inode;
}

static inode_t *vfs_create_file(fs_t *fs, inode_t *parent, const char *name)
{
    UNUSED(fs);
    CTASSERT(inode_is(parent, eNodeDir));

    vfs_dir_t *dir = inode_data(parent);
    inode_t *inode = vfs_new_file(parent, name);

    map_set(dir->children, name, inode);

    return inode;
}

static void vfs_delete_dir(fs_t *fs, inode_t *parent, const char *name)
{
    UNUSED(fs);
    CTASSERT(inode_is(parent, eNodeDir));

    vfs_dir_t *dir = inode_data(parent);
    map_delete(dir->children, name);
}

static void vfs_delete_file(fs_t *fs, inode_t *parent, const char *name)
{
    UNUSED(fs);
    CTASSERT(inode_is(parent, eNodeDir));

    vfs_dir_t *dir = inode_data(parent);
    map_delete(dir->children, name);
}

static io_t *vfs_file(fs_t *fs, inode_t *inode, file_flags_t flags)
{
    UNUSED(fs);
    UNUSED(flags);

    CTASSERT(inode_is(inode, eNodeFile));

    vfs_file_t *file = inode_data(inode);
    return io_virtual(file, "TODO", flags);
}

static map_t *vfs_dir(fs_t *fs, inode_t *inode)
{
    UNUSED(fs);
    CTASSERT(inode_is(inode, eNodeDir));

    vfs_dir_t *dir = inode_data(inode);
    return dir->children;
}

static const fs_callbacks_t kVirtual = {
    .fnQueryNode = vfs_query,

    .fnCreateDir = vfs_create_dir,
    .fnCreateFile = vfs_create_file,

    .fnDeleteDir = vfs_delete_dir,
    .fnDeleteFile = vfs_delete_file,

    .fnGetHandle = vfs_file,
    .fnGetDir = vfs_dir
};

fs_t *fs_virtual(const char *name)
{
    fs_virtual_t fs = {
        .name = name
    };

    inode_t *root = vfs_new_dir(NULL);

    return fs_new(&kVirtual, ".", root, &fs, sizeof(fs_virtual_t));
}
