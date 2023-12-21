#include "common.h"

#include "core/macros.h"

#include "base/panic.h"
#include "memory/arena.h"

#include "std/map.h"

#include "io/io.h"
#include "io/impl.h"

#include <string.h>

typedef struct virtual_t
{
    const char *name;
} virtual_t;

typedef struct virtual_file_t
{
    const char *name;

    // TODO: mutex

    char *data;
    size_t used;
    size_t size;
} virtual_file_t;

typedef struct virtual_dir_t
{
    map_t *dirents; ///< map<const char *, inode_t *>
} virtual_dir_t;

// io impl

typedef struct virtual_io_t
{
    virtual_file_t *data;
    size_t offset;
} virtual_io_t;

static virtual_io_t *vfs_data(io_t *self)
{
    return io_data(self);
}

static size_t vfs_read(io_t *self, void *dst, size_t size)
{
    virtual_io_t *io = vfs_data(self);
    size_t len = MIN(size, io->data->used - io->offset);
    memcpy(dst, io->data->data + io->offset, len);
    io->offset += len;
    return len;
}

static size_t vfs_write(io_t *self, const void *src, size_t size)
{
    virtual_io_t *io = vfs_data(self);
    virtual_file_t *data = io->data;

    data->used = MAX(data->used, io->offset + size);
    if (io->offset + size > data->size)
    {
        size_t new_size = MAX(data->size * 2, io->offset + size);
        data->data = arena_realloc(data->data, new_size, data->size, self->arena);
        data->size = new_size;
    }

    memcpy(data->data + io->offset, src, size);
    io->offset += size;

    return size;
}

static size_t vfs_size(io_t *self)
{
    virtual_io_t *io = vfs_data(self);
    return io->data->used;
}

static size_t vfs_seek(io_t *self, size_t offset)
{
    virtual_io_t *io = vfs_data(self);
    io->offset = MIN(offset, io->data->used);
    return io->offset;
}

static const void *vfs_map(io_t *self)
{
    virtual_io_t *io = vfs_data(self);
    return io->data->data;
}

static void vfs_close(io_t *self)
{
    CTU_UNUSED(self);
    /* empty */
}

static const io_callbacks_t kVirtualCallbacks = {
    .fn_read = vfs_read,
    .fn_write = vfs_write,

    .fn_get_size = vfs_size,
    .fn_seek = vfs_seek,

    .fn_map = vfs_map,
    .fn_close = vfs_close
};

static io_t *vfs_io(virtual_file_t *file, os_access_t flags, arena_t *arena)
{
    virtual_io_t io = {
        .data = file,
        .offset = 0
    };

    return io_new(&kVirtualCallbacks, flags, file->name, &io, sizeof(virtual_io_t), arena);
}

// fs impl

static inode_t *virtual_dir(arena_t *arena)
{
    virtual_dir_t dir = {
        .dirents = map_new(64)
    };

    return inode_dir(&dir, sizeof(virtual_dir_t), arena);
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
    return vfs_io(file, flags, fs->arena);
}

static inode_t *vfs_create_dir(fs_t *fs, inode_t *self, const char *name)
{
    CTU_UNUSED(fs);

    virtual_dir_t *dir = inode_data(self);
    inode_t *node = virtual_dir(fs->arena);
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

        .data = ARENA_MALLOC(fs->arena, 0x1000, "virtual_file", self),
        .used = 0,
        .size = 0x1000
    };

    inode_t *node = inode_file(&file, sizeof(virtual_file_t), fs->arena);
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

fs_t *fs_virtual(const char *name, arena_t *arena)
{
    CTASSERT(name != NULL);

    virtual_t self = {
        .name = name
    };

    inode_t *root = virtual_dir(arena);

    return fs_new(root, &kVirtualInterface, &self, sizeof(virtual_t), arena);
}
