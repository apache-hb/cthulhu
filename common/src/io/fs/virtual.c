#include "common.h"
#include "io/common.h"

#include "std/map.h"

#include "report/report.h"

#include "base/panic.h"
#include "base/memory.h"
#include "base/macros.h"

#include <string.h>

typedef struct virtual_t {
    const char *name;
} virtual_t;

typedef struct virtual_file_t {
    const char *name;

    // TODO: mutex

    char *data;
    size_t used;
    size_t size;
} virtual_file_t;

typedef struct virtual_dir_t {
    map_t *dirents; ///< map<const char *, inode_t *>
} virtual_dir_t;

// io impl

typedef struct virtual_io_t {
    virtual_file_t *data;
    size_t offset;
} virtual_io_t;

static size_t vfs_read(io_t *self, void *dst, size_t size)
{
    virtual_io_t *io = io_data(self);
    size_t len = MIN(size, io->data->used - io->offset);
    memcpy(dst, io->data->data + io->offset, len);
    io->offset += len;
    return len;
}

static size_t vfs_write(io_t *self, const void *src, size_t size)
{
    virtual_io_t *io = io_data(self);
    virtual_file_t *data = io->data;
    data->used = MAX(data->used, io->offset + size);
    if (io->offset + size > data->size)
    {
        data->data = ctu_realloc(data->data, io->offset + size);
        data->size = io->offset + size;
    }

    memcpy(data->data + io->offset, src, size);
    io->offset += size;

    return size;
}

static size_t vfs_size(io_t *self)
{
    virtual_io_t *io = io_data(self);
    return io->data->used;
}

static size_t vfs_seek(io_t *self, size_t offset)
{
    virtual_io_t *io = io_data(self);
    io->offset = MIN(offset, io->data->used);
    return io->offset;
}

static const void *vfs_map(io_t *self)
{
    virtual_io_t *io = io_data(self);
    return io->data->data;
}

static void vfs_close(io_t *self)
{
    CTU_UNUSED(self);
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

static io_t *vfs_io(virtual_file_t *file, os_access_t flags)
{
    virtual_io_t io = {
        .data = file,
        .offset = 0
    };

    return io_new(&kVirtualCallbacks, flags, file->name, &io, sizeof(virtual_io_t));
}

// fs impl

static inode_t *virtual_dir(void)
{
    virtual_dir_t dir = {
        .dirents = map_new(64)
    };

    return inode_dir(&dir, sizeof(virtual_dir_t));
}

static inode_t *vfs_query_node(fs_t *fs, inode_t *self, const char *name)
{
    CTU_UNUSED(fs);

    virtual_dir_t *dir = inode_data(self);
    return map_get_default(dir->dirents, name, &gInvalidINode);
}

static map_t *vfs_query_dirents(fs_t *fs, inode_t *self)
{
    CTU_UNUSED(fs);

    virtual_dir_t *dir = inode_data(self);
    return dir->dirents;
}

static io_t *vfs_query_file(fs_t *fs, inode_t *self, os_access_t flags)
{
    CTU_UNUSED(fs);

    virtual_file_t *file = inode_data(self);
    return vfs_io(file, flags);
}

static inode_t *vfs_create_dir(fs_t *fs, inode_t *self, const char *name)
{
    CTU_UNUSED(fs);

    virtual_dir_t *dir = inode_data(self);
    inode_t *node = virtual_dir();
    map_set(dir->dirents, name, node);
    return node;
}

static void vfs_delete_dir(fs_t *fs, inode_t *self, const char *name)
{
    CTU_UNUSED(fs);

    virtual_dir_t *dir = inode_data(self);
    map_delete(dir->dirents, name);
}

static inode_t *vfs_create_file(fs_t *fs, inode_t *self, const char *name)
{
    CTU_UNUSED(fs);

    virtual_dir_t *dir = inode_data(self);

    virtual_file_t file = {
        .name = name,

        .data = ctu_malloc(0x1000),
        .used = 0,
        .size = 0x1000
    };

    inode_t *node = inode_file(&file, sizeof(virtual_file_t));
    map_set(dir->dirents, name, node);
    return node;
}

static void vfs_delete_file(fs_t *fs, inode_t *self, const char *name)
{
    CTU_UNUSED(fs);

    virtual_dir_t *dir = inode_data(self);
    map_delete(dir->dirents, name);
}

static const fs_callbacks_t kVirtualInterface = {
    .pfn_query_node = vfs_query_node,
    .pfn_query_dirents = vfs_query_dirents,
    .pfn_query_file = vfs_query_file,

    .pfn_create_dir = vfs_create_dir,
    .pfn_delete_dir = vfs_delete_dir,

    .pfn_create_file = vfs_create_file,
    .pfn_delete_file = vfs_delete_file
};

fs_t *fs_virtual(reports_t *reports, const char *name)
{
    CTASSERT(name != NULL);

    virtual_t self = {
        .name = name
    };

    inode_t *root = virtual_dir();

    return fs_new(reports, root, &kVirtualInterface, &self, sizeof(virtual_t));
}
