#pragma once

#include "platform/error.h"
#include "base/macros.h"

typedef struct
{
    void *handle;
    const char *path;
} file_t;

typedef enum
{
    FILE_READ = (1 << 0),
    FILE_WRITE = (1 << 1),
    FILE_BINARY = (1 << 2),
    FILE_TEXT = (1 << 3),
} file_flags_t;

NODISCARD
file_t file_open(const char *path, file_flags_t flags, cerror_t *error);
void file_close(file_t file);

size_t file_read(file_t file, void *buffer, size_t size, cerror_t *error);
size_t file_write(file_t file, const void *buffer, size_t size, cerror_t *error);

NODISCARD
size_t file_size(file_t file, cerror_t *error);

NODISCARD
const void *file_map(file_t file, cerror_t *error);
