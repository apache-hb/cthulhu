#include "common.h"
#include "io/common.h"
#include "virtual.h"

#include "base/panic.h"

#include "std/str.h"

typedef struct vfs_io_t
{
    io_t *underlying;
} vfs_io_t;

static size_t vfs_read(io_t *self, void *dst, size_t size)
{
    vfs_io_t *vfs = io_data(self);
    return io_read(vfs->underlying, dst, size);
}

static size_t vfs_write(io_t *self, const void *src, size_t size)
{
    vfs_io_t *vfs = io_data(self);
    return io_write(vfs->underlying, src, size);
}

static size_t vfs_size(io_t *self)
{
    vfs_io_t *vfs = io_data(self);
    return io_size(vfs->underlying);
}

static const void *vfs_map(io_t *self)
{
    vfs_io_t *vfs = io_data(self);
    return io_map(vfs->underlying);
}

static void vfs_close(io_t *self) 
{ 

}

static const io_callbacks_t kVirtualFileCallbacks = {
    .fnRead = vfs_read,
    .fnWrite = vfs_write,

    .fnGetSize = vfs_size,

    .fnMap = vfs_map,
    .fnClose = vfs_close
};

io_t *io_virtual(vfs_file_t *file, const char *name, file_flags_t flags)
{
    io_t *data = io_blob(name, 0x1000);
    vfs_io_t virtualFile = {
        .underlying = data
    };

    return io_new(&kVirtualFileCallbacks, flags, format("%s - underlying", name), &virtualFile, sizeof(vfs_io_t));
}
