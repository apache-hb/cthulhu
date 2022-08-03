#pragma once

#include "platform/file.h"

#include "base/analyze.h"

typedef struct io_t io_t;
typedef size_t io_error_t;

void io_close(io_t *io);

NODISCARD ALLOC(io_close)
io_t *io_file(const char *path, file_flags_t mode);

NODISCARD ALLOC(io_close)
io_t *io_memory(const char *name, const void *data, size_t size);

NODISCARD ALLOC(io_close)
io_t *io_view(const char *name, const void *data, size_t size);

NODISCARD ALLOC(io_close)
io_t *io_string(const char *name, const char *string);

size_t io_read(io_t *io, void *dst, size_t size);
size_t io_write(io_t *io, IN_READS(size) const void *src, size_t size);

NODISCARD
const char *io_name(io_t *io);

NODISCARD
size_t io_size(io_t *io);

NODISCARD
const void *io_map(io_t *io);

NODISCARD MUST_INSPECT io_error_t io_error(const io_t *io);
