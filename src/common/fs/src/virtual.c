// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

#include "arena/arena.h"
#include "os/os.h"

#include "io/impl.h"
#include "std/map.h"

#include "base/util.h"
#include "base/panic.h"

#include "core/macros.h"

typedef struct virtual_t
{
    const char *name;
} virtual_t;

typedef struct virtual_file_t
{
    // TODO: mutex

    char *data;
    size_t used;
    size_t size;
} virtual_file_t;

typedef struct virtual_dir_t
{
    map_t *dirents; ///< map<const char *, fs_inode_t *>
} virtual_dir_t;

typedef union virtual_inode_t
{
    virtual_file_t file;
    virtual_dir_t dir;
} virtual_inode_t;

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
    size_t len = CT_MIN(size, io->data->used - io->offset);
    ctu_memcpy(dst, io->data->data + io->offset, len);
    io->offset += len;
    return len;
}

static size_t vfs_write(io_t *self, const void *src, size_t size)
{
    virtual_io_t *io = vfs_data(self);
    virtual_file_t *data = io->data;

    data->used = CT_MAX(data->used, io->offset + size);
    if (io->offset + size > data->size)
    {
        size_t new_size = CT_MAX(data->size * 2, io->offset + size);
        data->data = arena_realloc(data->data, new_size, data->size, self->arena);
        data->size = new_size;
    }

    ctu_memcpy(data->data + io->offset, src, size);
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
    io->offset = CT_MIN(offset, io->data->used);
    return io->offset;
}

static void *vfs_map(io_t *self, os_protect_t protect)
{
    CTASSERTF(!(protect & eOsProtectExecute), "cannot map vfs io object as executable `%s`", io_name(self));

    virtual_io_t *io = vfs_data(self);
    return io->data->data;
}

static os_error_t vfs_close(io_t *self)
{
    CT_UNUSED(self);
    return eOsSuccess;
}

static const io_callbacks_t kVirtualCallbacks = {
    .fn_read = vfs_read,
    .fn_write = vfs_write,

    .fn_get_size = vfs_size,
    .fn_seek = vfs_seek,

    .fn_map = vfs_map,
    .fn_close = vfs_close,

    .size = sizeof(virtual_io_t),
};

static io_t *vfs_io(fs_inode_t *inode, os_access_t flags, arena_t *arena)
{
    virtual_file_t *file = inode_data(inode);
    const char *name = inode_name(inode);

    virtual_io_t data = {
        .data = file,
        .offset = 0
    };

    if (flags & eOsAccessTruncate)
    {
        file->used = 0;
    }

    return io_new(&kVirtualCallbacks, flags, name, &data, arena);
}

// fs impl

static fs_inode_t *virtual_dir(fs_t *fs, const char *name)
{
    virtual_dir_t dir = {
        .dirents = map_optimal(64, kTypeInfoString, fs->arena)
    };

    virtual_inode_t inode = {
        .dir = dir
    };

    return inode_dir(fs, name, &inode);
}

static fs_inode_t *vfs_query_node(fs_t *fs, const fs_inode_t *self, const char *name)
{
    CT_UNUSED(fs);

    if (ctu_strlen(name) == 0) return (fs_inode_t*)self;
    if (ctu_strlen(name) == 1)
    {
        if (name[0] == '.') return (fs_inode_t*)self;
        if (name[0] == '/') return (fs_inode_t*)self;
    }

    virtual_dir_t *dir = inode_data((fs_inode_t*)self);
    return map_get_default(dir->dirents, name, &gInvalidFileNode);
}

static io_t *vfs_query_file(fs_t *fs, fs_inode_t *self, os_access_t flags)
{
    return vfs_io(self, flags, fs->arena);
}

static inode_result_t vfs_create_dir(fs_t *fs, fs_inode_t *self, const char *name)
{
    virtual_dir_t *dir = inode_data(self);
    fs_inode_t *node = virtual_dir(fs, name);
    map_set(dir->dirents, name, node);

    inode_result_t result = { .node = node };
    return result;
}

static os_error_t vfs_delete_dir(fs_t *fs, fs_inode_t *self, const char *name)
{
    CT_UNUSED(fs);

    virtual_dir_t *dir = inode_data(self);
    bool result = map_delete(dir->dirents, name);
    return result ? eOsSuccess : eOsNotFound;
}

static inode_result_t vfs_create_file(fs_t *fs, fs_inode_t *parent, const char *name)
{
    virtual_dir_t *dir = inode_data(parent);

    virtual_file_t file = {
        .data = ARENA_MALLOC(0x1000, "data", parent, fs->arena),
        .used = 0,
        .size = 0x1000
    };

    virtual_inode_t inode = {
        .file = file
    };

    fs_inode_t *node = inode_file(fs, name, &inode);
    ARENA_REPARENT(node, parent, fs->arena);
    ARENA_REPARENT(file.data, node, fs->arena);
    map_set(dir->dirents, name, node);
    inode_result_t result = { .node = node };
    return result;
}

static os_error_t vfs_delete_file(fs_t *fs, fs_inode_t *self, const char *name)
{
    CT_UNUSED(fs);

    virtual_dir_t *dir = inode_data(self);
    bool result = map_delete(dir->dirents, name);
    return result ? eOsSuccess : eOsNotFound;
}

static os_error_t vfs_iter_begin(fs_t *fs, const fs_inode_t *dir, fs_iter_t *iter)
{
    CT_UNUSED(fs);

    virtual_dir_t *data = inode_data((fs_inode_t*)dir);
    map_iter_t *result = iter_data(iter);
    *result = map_iter(data->dirents);

    return eOsSuccess;
}

static os_error_t vfs_iter_next(fs_iter_t *iter)
{
    map_iter_t *it = iter_data(iter);
    if (!map_has_next(it))
        return eOsNotFound;

    map_entry_t entry = map_next(it);
    iter->current = entry.value;

    return eOsSuccess;
}

static os_error_t vfs_iter_end(fs_iter_t *iter)
{
    CT_UNUSED(iter);
    return eOsSuccess;
}

static const fs_callbacks_t kVirtualInterface = {
    .pfn_query_node = vfs_query_node,
    .pfn_query_file = vfs_query_file,

    .pfn_create_dir = vfs_create_dir,
    .pfn_delete_dir = vfs_delete_dir,

    .pfn_create_file = vfs_create_file,
    .pfn_delete_file = vfs_delete_file,

    .pfn_iter_begin = vfs_iter_begin,
    .pfn_iter_next = vfs_iter_next,
    .pfn_iter_end = vfs_iter_end,

    .iter_size = sizeof(map_iter_t),
    .inode_size = sizeof(virtual_inode_t)
};

STA_DECL
fs_t *fs_virtual(const char *name, arena_t *arena)
{
    CTASSERT(name != NULL);

    virtual_t self = {
        .name = name
    };

    virtual_dir_t dir = {
        .dirents = map_optimal(64, kTypeInfoString, arena),
    };

    virtual_inode_t inode = {
        .dir = dir
    };

    return fs_new(&inode, &kVirtualInterface, &self, sizeof(virtual_t), arena);
}
