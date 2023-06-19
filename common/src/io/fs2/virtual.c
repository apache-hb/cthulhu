#include "common.h"

#include "std/map.h"

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

static inode2_t *vfs_query_node(inode2_t *self, const char *name)
{
    virtual_dir_t *dir = inode2_data(self);
    return map_get(dir->dirents, name);
}

static const fs2_interface_t kVirtualInterface = {
    .name = "Virtual File System"
};

fs2_t *fs2_virtual(reports_t *reports, const char *name)
{
    virtual_t self = {
        .name = name
    };

    inode2_t *root = virtual_dir();

    return fs2_new(reports, root, &kVirtualInterface, &self, sizeof(virtual_t));
}
