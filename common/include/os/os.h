#pragma once

#include "os/error.h"

#include <stdbool.h>

// opaque types

typedef struct os_file_t os_file_t;
typedef struct os_dir_t os_dir_t;
typedef struct os_iter_t os_iter_t;

typedef enum os_mode_t
{
    eAccessRead = (1 << 0), // file is readable
    eAccessWrite = (1 << 1), // file is writeable, does not imply readable

    eAccessText = (1 << 2), ///< enable eof translation
} os_access_t;

typedef enum os_dirent_t
{
    eOsNodeNone,
    eOsNodeFile,
    eOsNodeDir,

    eOsNodeTotal
} os_dirent_t;

///
/// global init
///

void os_init(void);

///
/// filesystem api
///

MUST_INSPECT
OS_RESULT(bool) os_file_create(const char *path);

MUST_INSPECT
OS_RESULT(bool) os_file_delete(const char *path);

MUST_INSPECT
OS_RESULT(bool) os_file_exists(const char *path);

MUST_INSPECT
OS_RESULT(bool) os_dir_create(const char *path);

MUST_INSPECT
OS_RESULT(bool) os_dir_delete(const char *path);

MUST_INSPECT
OS_RESULT(bool) os_dir_exists(const char *path);

NODISCARD
OS_RESULT(bool) os_dirent_exists(const char *path);

NODISCARD
OS_RESULT(os_dirent_t) os_dirent_type(const char *path);

NODISCARD
OS_RESULT(const char *) os_dir_current(void);

///
/// directory api
///

/**
 * @brief open a directory for iteration
 *
 * @param path path to directory
 * @return result containing either a valid iterator or an error, NULL if dir does not exist
 */
NODISCARD
OS_RESULT(os_iter_t) os_iter_begin(const char *path);

void os_iter_end(os_iter_t *iter);

/**
 * @brief get the next directory entry
 *
 * @param iter iterator to use
 * @return result containing either a valid directory entry or an error, NULL if no more entries
 */
NODISCARD
OS_RESULT(os_dir_t) os_iter_next(os_iter_t *iter);

NODISCARD
const char *os_dir_name(os_dir_t *dir);

///
/// file api
///

NODISCARD
OS_RESULT(os_file_t *) os_file_open(const char *path, os_access_t access);

void os_file_close(os_file_t *file);

NODISCARD
OS_RESULT(size_t) os_file_read(os_file_t *file, void *buffer, size_t size);

NODISCARD
OS_RESULT(size_t) os_file_write(os_file_t *file, const void *buffer, size_t size);

NODISCARD
OS_RESULT(size_t) os_file_size(os_file_t *file);

NODISCARD
OS_RESULT(size_t) os_file_seek(os_file_t *file, size_t offset);

NODISCARD
OS_RESULT(size_t) os_file_tell(os_file_t *file);

NODISCARD
OS_RESULT(const void *) os_file_map(os_file_t *file);

NODISCARD
const char *os_file_name(os_file_t *file);
