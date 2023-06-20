#include "common.h"

#include "std/map.h"

#include "report/report.h"

#include "base/panic.h"

typedef struct virtual_t
{
    const char *name;
} virtual_t;

typedef struct virtual_file_t
{
    io_t *io;
} virtual_file_t;

typedef struct virtual_dir_t
{
    map_t *dirents; ///< map<const char *, inode2_t *>
} virtual_dir_t;

static inode2_t *virtual_dir(void)
{
    virtual_dir_t dir = {
        .dirents = map_new(64)
    };
    
    return inode2_dir(&dir, sizeof(virtual_dir_t));
}

static inode2_t *vfs_query_node(fs2_t *fs, inode2_t *self, const char *name)
{
    UNUSED(fs);

    virtual_dir_t *dir = inode2_data(self);
    return map_get_default(dir->dirents, name, &kInvalidINode);
}

static map_t *vfs_query_dirents(fs2_t *fs, inode2_t *self)
{
    UNUSED(fs);

    virtual_dir_t *dir = inode2_data(self);
    return dir->dirents;
}

static inode2_t *vfs_create_dir(fs2_t *fs, inode2_t *self, const char *name)
{
    UNUSED(fs);

    virtual_dir_t *dir = inode2_data(self);
    inode2_t *node = virtual_dir();
    map_set(dir->dirents, name, node);
    return node;
}

static void vfs_delete_dir(fs2_t *fs, inode2_t *self, const char *name)
{
    UNUSED(fs);

    virtual_dir_t *dir = inode2_data(self);
    map_delete(dir->dirents, name);
}

static const fs2_interface_t kVirtualInterface = {
    .name = "Virtual File System",

    .fnQueryNode = vfs_query_node,
    .fnQueryDirents = vfs_query_dirents,

    .fnCreateDir = vfs_create_dir,
    .fnDeleteDir = vfs_delete_dir
};

fs2_t *fs2_virtual(reports_t *reports, const char *name)
{
    virtual_t self = {
        .name = name
    };

    inode2_t *root = virtual_dir();

    return fs2_new(reports, root, &kVirtualInterface, &self, sizeof(virtual_t));
}
