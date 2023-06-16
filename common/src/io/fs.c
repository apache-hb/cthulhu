#include "io/fs.h"

#include "base/memory.h"
#include "base/panic.h"

#include "std/map.h"
#include "std/str.h"

#include <string.h>

typedef io_t *(*fs_open_t)(fs_t *self, const char *name, file_flags_t flags);
typedef void (*fs_mkdir_t)(fs_t *self, const char *path);

typedef void (*fs_close_t)(fs_t *self);

typedef struct fs_callbacks_t
{
    fs_open_t fnOpenFile;
    fs_mkdir_t fnNewDir;

    fs_close_t fnClose;
} fs_callbacks_t;

typedef enum vfs_file_type_t
{
    eNodeFile,
    eNodeDir,

    eNodeTotal
} vfs_file_type_t;

typedef struct vfs_inode_t
{
    vfs_file_type_t type;
    struct vfs_inode_t *parent;

    union
    {
        io_t *file;
        map_t *dir;
    };
} vfs_inode_t;

typedef struct fs_virtual_t
{
    const char *name;
    map_t *files;
} fs_virtual_t;

typedef struct fs_physical_t
{
    const char *root;
} fs_physical_t;

typedef struct fs_t 
{
    const fs_callbacks_t *cb;
    char data[];
} fs_t;

// internal helpers

static void *fs_data(fs_t *fs)
{
    return fs->data;
}

// virtual file system

static vfs_inode_t *inode_new(vfs_file_type_t type, vfs_inode_t *parent)
{
    vfs_inode_t *inode = ctu_malloc(sizeof(vfs_inode_t));
    inode->type = type;
    inode->parent = parent;

    return inode;
}

static vfs_inode_t *inode_file(vfs_inode_t *parent, io_t *file)
{
    vfs_inode_t *inode = inode_new(eNodeFile, parent);
    inode->file = file;

    return inode;
}

static vfs_inode_t *inode_dir(vfs_inode_t *parent)
{
    vfs_inode_t *inode = inode_new(eNodeDir, parent);
    inode->dir = map_optimal(32);

    return inode;
}

static bool inode_is(vfs_inode_t *inode, vfs_file_type_t type)
{
    return inode != NULL && inode->type == type;
}

static io_t *vfs_open_file(fs_t *fs, const char *name, file_flags_t flags)
{
    fs_virtual_t *vfs = fs_data(fs);

    vfs_inode_t *existing = map_get(vfs->files, name);

    if (inode_is(existing, eNodeFile)) 
    { 
        return existing->file; 
    }

    io_t *io = io_blob(format("vfs(%s) - %s", vfs->name, name), 0x1000);
    vfs_inode_t *inode = inode_file(existing, io);

    map_set(vfs->files, name, inode);

    return io;
}

// physical file system

static io_t *pfs_open_file(fs_t *fs, const char *name, file_flags_t flags)
{
    fs_physical_t *pfs = fs_data(fs);

    char *path = format("%s" NATIVE_PATH_SEPARATOR "%s", pfs->root, name);
    return io_file(path, flags);
}

// callbacks

static const fs_callbacks_t kVirtual = {
    .fnOpenFile = vfs_open_file
};

static const fs_callbacks_t kPhysical = {
    .fnOpenFile = pfs_open_file
};

static fs_t *fs_new(const fs_callbacks_t *cb, void *data, size_t size)
{
    fs_t *fs = ctu_malloc(sizeof(fs_t) + size);
    fs->cb = cb;
    memcpy(fs->data, data, size);

    return fs;
}

fs_t *fs_virtual(const char *name)
{
    fs_virtual_t fs = {
        .name = name,
        .files = map_optimal(32)
    };

    return fs_new(&kVirtual, &fs, sizeof(fs_virtual_t));
}

fs_t *fs_physical(const char *root)
{
    fs_physical_t fs = {
        .root = root
    };

    return fs_new(&kPhysical, &fs, sizeof(fs_physical_t));
}

io_t *fs_open(fs_t *fs, const char *path, file_flags_t mode)
{
    CTASSERT(fs != NULL);
    CTASSERT(fs->cb != NULL && fs->cb->fnOpenFile != NULL);
    CTASSERT(path != NULL);

    return fs->cb->fnOpenFile(fs, path, mode);
}
