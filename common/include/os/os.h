#pragma once

#include <stdbool.h>

// opaque types

typedef struct os_file_t os_file_t;
typedef struct os_dir_t os_dir_t;
typedef struct os_iter_t os_iter_t;
typedef struct os_result_t os_result_t;

typedef enum os_mode_t
{
    eAccessRead = (1 << 0), // file is readable
    eAccessWrite = (1 << 1), // file is writeable, does not imply readable

    eAccessText = (1 << 2), ///< enable eof translation
} os_access_t;

typedef enum os_dirent_type_t
{
    eOsNodeFile,
    eOsNodeDir,

    eOsNodeTotal
} os_dirent_type_t;

// documentation macros

#define OS_RESULT(TYPE) os_result_t *

// result & error api

bool os_result_valid(const os_result_t *result);
void *os_result_value(os_result_t *result);
const char *os_result_error(os_result_t *result);

// filesystem api

OS_RESULT(bool) os_file_create(const char *path);
OS_RESULT(bool) os_file_delete(const char *path);
OS_RESULT(bool) os_file_exists(const char *path);

OS_RESULT(bool) os_dir_create(const char *path);
OS_RESULT(bool) os_dir_delete(const char *path);
OS_RESULT(bool) os_dir_exists(const char *path);

OS_RESULT(bool) os_dirent_exists(const char *path);
OS_RESULT(os_dirent_t) os_dirent_type(const char *path);

OS_RESULT(const char *) os_dir_current(void);

// directory api

/**
 * @brief open a directory for iteration
 * 
 * @param path path to directory
 * @return result containing either a valid iterator or an error, NULL if dir does not exist
 */
OS_RESULT(os_iter_t *) os_iter_begin(const char *path);

void os_iter_end(os_iter_t *iter);

/**
 * @brief get the next directory entry
 * 
 * @param iter iterator to use
 * @return result containing either a valid directory entry or an error, NULL if no more entries
 */
OS_RESULT(os_dir_t *) os_iter_next(os_iter_t *iter);

const char *os_dir_name(os_dir_t *dir);

// file api

OS_RESULT(os_file_t *) os_file_open(const char *path, os_access_t access);

OS_RESULT(void) os_file_close(os_file_t *file);

OS_RESULT(size_t) os_file_read(os_file_t *file, void *buffer, size_t size);
OS_RESULT(size_t) os_file_write(os_file_t *file, const void *buffer, size_t size);

OS_RESULT(size_t) os_file_size(os_file_t *file);
OS_RESULT(size_t) os_file_seek(os_file_t *file, size_t offset);
OS_RESULT(size_t) os_file_tell(os_file_t *file);

OS_RESULT(const void *) os_file_map(os_file_t *file);
OS_RESULT(const char *) os_file_name(os_file_t *file);
