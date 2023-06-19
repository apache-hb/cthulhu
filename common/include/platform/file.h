#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "platform/error.h"

typedef struct
{
    void *handle;
    const char *path;
} file_t;

typedef enum
{
    eFileRead = (1 << 0),
    eFileWrite = (1 << 1),
    eFileBinary = (1 << 2),
    eFileText = (1 << 3),
} file_flags_t;

NODISCARD
cerror_t make_directory(const char *path);

NODISCARD
void delete_directory(const char *path);

NODISCARD
cerror_t delete_file(const char *path);

NODISCARD
bool is_directory(const char *path);

NODISCARD
bool is_file(const char *path);

void file_close(file_t file);

NODISCARD
file_t file_open(const char *path, file_flags_t flags, cerror_t *error);

size_t file_read(file_t file, IN_NOTNULL void *buffer, size_t size, cerror_t *error);
size_t file_write(file_t file, IN_READS(size) const void *buffer, size_t size, cerror_t *error);

NODISCARD
bool file_valid(file_t file);

NODISCARD
size_t file_size(file_t file, cerror_t *error);

NODISCARD
const void *file_map(file_t file, cerror_t *error);
