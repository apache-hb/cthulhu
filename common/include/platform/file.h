#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "platform/error.h"

typedef struct
{
    void *handle;
    const char *path;
} file_t;

typedef struct dir_iter_t
{
    void *handle;
    cerror_t error;
} dir_iter_t;

typedef struct
{
    const char *name;
} dir_t;

typedef enum
{
    eFileRead = (1 << 0),
    eFileWrite = (1 << 1),
    eFileBinary = (1 << 2),
    eFileText = (1 << 3),
} file_flags_t;

NODISCARD
cerror_t dir_create(const char *path);

NODISCARD
void dir_delete(const char *path);

NODISCARD
const char *get_cwd(void);

NODISCARD
cerror_t file_delete(const char *path);

NODISCARD
bool dir_exists(const char *path);

NODISCARD
bool file_exists(const char *path);

// dir api

void dir_close(dir_iter_t *iter);

dir_iter_t dir_open(const char *path);
bool dir_next(dir_iter_t *iter, dir_t *dir);

// file api

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
size_t file_seek(file_t file, size_t offset, cerror_t *error);

NODISCARD
const void *file_map(file_t file, cerror_t *error);
