#pragma once

#include "macros.h"
#include "util.h"

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

/**
 * @defgroup FileApi File stream API
 * @brief file and memory stream API
 * @{
 */

#ifdef _WIN32
#    define PATH_SEP "\\" ///< path separator
#else
#    define PATH_SEP "/" ///< path separator
#endif

/**
 * @brief the path a file without its extension
 *
 * @param path the path to the file
 * @return the path without the extension
 */
char *ctu_noext(const char *path);

/**
 * @brief the name of a file without its path
 *
 * @param path the path to the file
 * @return the name of the file without its path
 */
char *ctu_filename(const char *path);

struct file_t;

/**
 * @defgroup FileApiCallbacks File stream callbacks
 * @ingroup FileApi
 * @brief file stream callbacks
 * @{
 */

/**
 * @brief file read callback
 *
 * @param self the generic file stream object
 * @param dst the destination buffer
 * @param total the number of bytes to read
 *
 * @return the number of bytes read
 */
typedef size_t (*file_read_t)(struct file_t *self, void *dst, size_t total);

/**
 * @brief file write callback
 *
 * @param self the generic file stream object
 * @param src the source buffer
 * @param total the number of bytes to write
 *
 * @return the number of bytes written
 */
typedef size_t (*file_write_t)(struct file_t *self, const void *src, size_t total);

/**
 * @brief file seek callback
 *
 * @param self the generic file stream object
 * @param offset the offset to seek to
 *
 * @return the new offset
 */
typedef size_t (*file_seek_t)(struct file_t *self, size_t offset);

/**
 * @brief file size callback
 *
 * @param self the generic file stream object
 *
 * @return the size of the file
 */
typedef size_t (*file_size_t)(struct file_t *self);

/**
 * @brief file tell callback
 *
 * @param self the generic file stream object
 *
 * @return the current offset
 */
typedef size_t (*file_tell_t)(struct file_t *self);

/**
 * @brief file map callback
 *
 * @param self the generic file stream object
 *
 * @return the mapped file or NULL on error
 */
typedef void *(*file_map_t)(struct file_t *self);

/**
 * @brief file ok callback
 *
 * @param self the generic file stream object
 *
 * @return true if the file is ok, false otherwise
 */
typedef bool (*file_ok_t)(struct file_t *self);

/** @} */

/**
 * @brief file operation callbacks
 */
typedef struct
{
    file_read_t read;   ///< read callback
    file_write_t write; ///< write callback
    file_seek_t seek;   ///< seek callback
    file_size_t size;   ///< size callback
    file_tell_t tell;   ///< tell callback
    file_map_t mapped;  ///< map callback
    file_ok_t ok;       ///< ok callback
} file_ops_t;

/**
 * @brief the type of contents in a file stream
 */
typedef enum
{
    BINARY, ///< binary file
    TEXT    ///< text file
} contents_t;

/**
 * @brief file permissions
 */
typedef enum
{
    READ = (1 << 0),  ///< read permission
    WRITE = (1 << 1), ///< write permission
    EXEC = (1 << 2)   ///< execute permission
} access_t;

/**
 * @brief the file backing type
 */
typedef enum
{
    FD,    ///< file descriptor
    MEMORY ///< in memory file
} file_type_t;

/**
 * @brief generic file stream
 */
typedef struct file_t
{
    const char *path;    ///< the path to the file
    contents_t format;   ///< the type of contents in the file
    access_t access;     ///< the permissions of the file
    file_type_t backing; ///< the type of stream

    const file_ops_t *ops; ///< file operation callbacks
    char data[];           ///< the internal file data
} file_t;

/**
 * @brief close a file handle
 *
 * @param file the file handle to close
 */
void close_file(file_t *file);

/**
 * @brief open a file
 *
 * @param path the path to the file
 * @param format the format of the files contents
 * @param access the desired permissions
 *
 * @return a file handle
 */
file_t *file_new(const char *path, contents_t format, access_t access);

/**
 * @brief open a memory stream
 *
 * @param name the name of the memory stream
 * @param size the initial size of the memory stream
 * @param format the format of the streams contents
 * @param access the desired permissions
 *
 * @return a file handle
 */
file_t *memory_new(const char *name, size_t size, contents_t format, access_t access);

/**
 * @brief read from a file
 *
 * @param file the file handle
 * @param dst the destination buffer
 * @param total the total number of bytes to read
 *
 * @return the number of bytes read
 */
size_t file_read(file_t *file, void *dst, size_t total);

/**
 * @brief write to a file
 *
 * @param file the file handle
 * @param src the source buffer
 * @param total the total number of bytes to write
 *
 * @return the number of bytes written
 */
size_t file_write(file_t *file, const void *src, size_t total);

/**
 * @brief seek to a position in a file
 *
 * @param file the file handle
 * @param offset the offset to seek to
 *
 * @return the new offset
 */
size_t file_seek(file_t *file, size_t offset);

/**
 * @brief get the size of a file
 *
 * @param file the file handle
 *
 * @return the size of the file
 */
size_t file_size(file_t *file);

/**
 * @brief get the current position of the file
 *
 * @param file the file handle
 *
 * @return the current position in the file
 */
size_t file_tell(file_t *file);

/**
 * @brief map a file into memory
 *
 * @param file the file to map
 *
 * @return the mapped memory or NULL if the mapping failed
 */
void *file_map(file_t *file);

/**
 * @brief check if a file handle is valid
 *
 * @param file the file handle to check
 *
 * @return true if the file handle is valid
 */
bool file_ok(file_t *file);

/**
 * @brief check if a file exists
 *
 * @param path the path to the file
 *
 * @return true if the file exists
 */
bool file_exists(const char *path);

/** @} */
