#include "common.h"
#include "io/common.h"

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

// io impl

static size_t vfs_read(io_t *self, void *dst, size_t size)
{
    virtual_file_t *file = io_data(self);
    return io_read(file->io, dst, size);
}

static size_t vfs_write(io_t *self, const void *src, size_t size)
{
    virtual_file_t *file = io_data(self);
    return io_write(file->io, src, size);
}

static size_t vfs_size(io_t *self)
{
    virtual_file_t *file = io_data(self);
    return io_size(file->io);
}

static size_t vfs_seek(io_t *self, size_t offset)
{
    virtual_file_t *file = io_data(self);
    return io_seek(file->io, offset);
}

static const void *vfs_map(io_t *self)
{
    virtual_file_t *file = io_data(self);
    return io_map(file->io);
}

static void vfs_close(io_t *self) 
{ 
    /* empty */
}

static const io_callbacks_t kVirtualCallbacks = {
    .fnRead = vfs_read,
    .fnWrite = vfs_write,

    .fnGetSize = vfs_size,
    .fnSeek = vfs_seek,

    .fnMap = vfs_map,
    .fnClose = vfs_close
};

static io_t *vfs_io(virtual_file_t *file, file_flags_t flags)
{
    return io_new(&kVirtualCallbacks, flags, io_name(file->io), file, sizeof(virtual_file_t));
}

// fs impl

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

static io_t *vfs_query_file(fs2_t *fs, inode2_t *self, file_flags_t flags)
{
    UNUSED(fs);

    virtual_file_t *file = inode2_data(self);
    return vfs_io(file, flags);
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

static inode2_t *vfs_create_file(fs2_t *fs, inode2_t *self, const char *name)
{
    UNUSED(fs);

    virtual_dir_t *dir = inode2_data(self);

    virtual_file_t file = {
        .io = io_blob(name, 0x1000, eFileRead | eFileWrite | eFileBinary)
    };

    inode2_t *node = inode2_file(&file, sizeof(virtual_file_t));
    map_set(dir->dirents, name, node);
    return node;
}

static void vfs_delete_file(fs2_t *fs, inode2_t *self, const char *name)
{
    UNUSED(fs);

    virtual_dir_t *dir = inode2_data(self);
    map_delete(dir->dirents, name);
}

static const fs2_interface_t kVirtualInterface = {
    .name = "Virtual File System",

    .fnQueryNode = vfs_query_node,
    .fnQueryDirents = vfs_query_dirents,
    .fnQueryFile = vfs_query_file,

    .fnCreateDir = vfs_create_dir,
    .fnDeleteDir = vfs_delete_dir,

    .fnCreateFile = vfs_create_file,
    .fnDeleteFile = vfs_delete_file
};

fs2_t *fs2_virtual(reports_t *reports, const char *name)
{
    virtual_t self = {
        .name = name
    };

    inode2_t *root = virtual_dir();

    return fs2_new(reports, root, &kVirtualInterface, &self, sizeof(virtual_t));
}
