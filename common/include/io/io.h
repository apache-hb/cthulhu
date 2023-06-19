#pragma once

#include "platform/file.h"

#include "base/analyze.h"
#include "base/macros.h"

BEGIN_API

typedef struct io_t io_t;
typedef size_t io_error_t;

STATIC_ASSERT(sizeof(io_error_t) == sizeof(cerror_t), "io error and cerror must be the same size");

void io_close(io_t *io);

NODISCARD ALLOC(io_close)
io_t *io_file(const char *path, file_flags_t mode);

NODISCARD ALLOC(io_close)
io_t *io_memory(const char *name, const void *data, size_t size);

NODISCARD ALLOC(io_close)
io_t *io_blob(const char *name, size_t size);

NODISCARD ALLOC(io_close)
io_t *io_view(const char *name, const void *data, size_t size);

NODISCARD ALLOC(io_close)
io_t *io_string(const char *name, const char *string);

size_t io_read(io_t *io, IN_NOTNULL void *dst, size_t size);
size_t io_write(io_t *io, IN_READS(size) const void *src, size_t size);

NODISCARD
const char *io_name(IN_NOTNULL io_t *io);

NODISCARD
size_t io_size(IN_NOTNULL io_t *io);

NODISCARD
size_t io_seek(IN_NOTNULL io_t *io, size_t offset);

NODISCARD
const void *io_map(IN_NOTNULL io_t *io);

NODISCARD MUST_INSPECT 
io_error_t io_error(IN_NOTNULL const io_t *io);

END_API
