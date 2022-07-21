#pragma once

#include "platform/file.h"

typedef struct io_t io_t;
typedef struct alloc_t alloc_t;
typedef size_t io_error_t;

io_t *io_file(alloc_t *alloc, const char *path, file_flags_t mode);
io_t *io_memory(alloc_t *alloc, const char *name, const void *data, size_t size);

size_t io_read(io_t *io, void *dst, size_t size);
size_t io_write(io_t *io, const void *src, size_t size);

size_t io_tell(io_t *io);
size_t io_seek(io_t *io, size_t offset);
size_t io_size(io_t *io);

const void *io_map(io_t *io);

io_error_t io_error(const io_t *io);
