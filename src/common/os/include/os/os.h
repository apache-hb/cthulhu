#pragma once

#include OS_API_HEADER

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdbool.h>

BEGIN_API

/// @defgroup OS Platform independent OS api
/// @ingroup Common
/// @{

/// @brief file handle
typedef struct os_file_t os_file_t;

/// @brief directory handle
typedef struct os_dir_t os_dir_t;

/// @brief directory iterator
typedef struct os_iter_t os_iter_t;

/// @brief file access mode
typedef enum os_mode_t
{
    eAccessRead = (1 << 0),  ///< file is readable
    eAccessWrite = (1 << 1), ///< file is writeable, does not imply readable

    eAccessText = (1 << 2), ///< enable eof translation
} os_access_t;

/// @brief directory entry type
typedef enum os_dirent_t
{
    eOsNodeNone, ///< unknown node type
    eOsNodeFile, ///< file node type
    eOsNodeDir,  ///< directory node type
    eOsNodeError,///< error node type

    eOsNodeTotal
} os_dirent_t;

/// @brief error code
typedef size_t os_error_t;

/// @brief convert an os error code to a string
///
/// @param error the error code to convert
///
/// @return the string representation of the error code
NODISCARD RET_STRING
const char *os_error_string(os_error_t error);

/// @brief initialize the os api
/// @note this must be called before using any other os api
void os_init(void);

/// filesytem api

/// @brief create a file
///
/// @param path the path to the file to create
///
/// @return true if the file was created, false if it already exists
/// @return an error if the file could not be created
RET_INSPECT
os_error_t os_file_create(IN_STRING const char *path);

/// @brief delete a file
///
/// @param path the path to the file to delete
///
/// @return true if the file was deleted, false if it did not exist
/// @return an error if the file could not be deleted
RET_INSPECT
os_error_t os_file_delete(IN_STRING const char *path);

/// @brief check if a directory exists
///
/// @param path the path to the directory to check
///
/// @return true if the directory exists, false otherwise
/// @return an error if the directory could not be checked
RET_INSPECT
os_error_t os_dir_create(IN_STRING const char *path, IN_NOTNULL bool *create);

/// @brief delete a directory
///
/// @param path the path to the directory to delete
///
/// @return true if the directory was deleted, false if it did not exist
/// @return an error if the directory could not be deleted
RET_INSPECT
os_error_t os_dir_delete(IN_STRING const char *path);

/// @brief check if a directory exists
///
/// @param path the path to the directory to check
///
/// @return true if the directory exists, false otherwise
/// @return an error if the directory could not be checked
RET_INSPECT
bool os_dir_exists(IN_STRING const char *path);

/// @brief get the type of a paths inode entry
///
/// @param path the path to the inode entry to check
///
/// @return the type of the inode entry
/// @return an error if the inode entry could not be checked
NODISCARD
os_dirent_t os_dirent_type(IN_STRING const char *path);

/// @brief get the current working directory
///
/// @return the current working directory
/// @return an error if the current working directory could not be retrieved
NODISCARD
os_error_t os_dir_current(IN_NOTNULL const char **cwd);

/// directory iteration

/// @brief open a directory for iteration
///
/// @param path path to directory
/// @return result containing either a valid iterator or an error, NULL if dir does not exist
/// @note the iterator must be closed with os_iter_end
NODISCARD
os_error_t os_iter_begin(IN_STRING const char *path, os_iter_t *iter);

/// @brief close a directory iterator
///
/// @param iter iterator to close
void os_iter_end(IN_NOTNULL os_iter_t *iter);

/// @brief get the next directory entry
///
/// @param iter iterator to use
/// @param dir directory entry to fill
///
/// @return true if a directory entry was found
NODISCARD
bool os_iter_next(IN_NOTNULL os_iter_t *iter, os_dir_t *dir);

os_error_t os_iter_error(IN_NOTNULL os_iter_t *iter);

/// @brief get the name of a directory entry
///
/// @param dir directory entry to get the name of
/// @return the name of the directory entry
NODISCARD
const char *os_dir_name(IN_NOTNULL os_dir_t *dir);

/// file api

/// @brief open a file
///
/// @param path the path to the file to open
/// @param access the access mode to open the file with
///
/// @return a file handle on success
/// @return an error if the file could not be opened
NODISCARD
os_error_t os_file_open(IN_STRING const char *path, os_access_t access, os_file_t *file);

/// @brief close a file
///
/// @param file the file to close
void os_file_close(OUT_PTR_INVALID os_file_t *file);

/// @brief read from a file
///
/// @param file the file to read from
/// @param buffer the buffer to read into
/// @param size the number of bytes to read
///
/// @return the number of bytes read
/// @return an error if the file could not be read from
NODISCARD
os_error_t os_file_read(
        IN_NOTNULL os_file_t *file,
        void *buffer,
        IN_RANGE(>, 0) size_t size,
        size_t *actual);

/// @brief write to a file
///
/// @param file the file to write to
/// @param buffer the buffer to write from
/// @param size the number of bytes to write
///
/// @return the number of bytes written
/// @return an error if the file could not be written to
NODISCARD
os_error_t os_file_write(
        IN_NOTNULL os_file_t *file,
        IN_READS(size) const void *buffer,
        IN_RANGE(>, 0) size_t size,
        size_t *actual);

/// @brief get the size of a file
///
/// @param file the file to get the size of
///
/// @return the size of the file
/// @return an error if the file size could not be retrieved
NODISCARD
os_error_t os_file_size(IN_NOTNULL os_file_t *file, size_t *actual);

/// @brief seek to a position in a file
///
/// @param file the file to seek in
/// @param offset the offset to seek to
///
/// @return the new position in the file
/// @return an error if the file could not be seeked
NODISCARD
os_error_t os_file_seek(IN_NOTNULL os_file_t *file, size_t offset, size_t *actual);

/// @brief get the current position in a file
///
/// @param file the file to get the position of
///
/// @return the current position in the file
/// @return an error if the file position could not be retrieved
NODISCARD
os_error_t os_file_tell(IN_NOTNULL os_file_t *file, size_t *actual);

/// @brief map a file into memory
///
/// @param file the file to map
///
/// @return a pointer to the mapped file
/// @return an error if the file could not be mapped
NODISCARD
os_error_t os_file_map(IN_NOTNULL os_file_t *file, const void **mapped);

/// @brief get the name of a file
///
/// @param file the file to get the name of
///
/// @return the name of the file
NODISCARD
const char *os_file_name(IN_NOTNULL os_file_t *file);

/// @}

END_API
