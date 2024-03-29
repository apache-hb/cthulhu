// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "os/core.h"

#if CT_OS_WINDOWS
#   include "impl/win32.h" // IWYU pragma: export
#elif CT_OS_LINUX || CT_OS_APPLE
#   include "impl/posix.h" // IWYU pragma: export
#else
#   error "unsupported platform"
#endif

#include <stdbool.h>

CT_BEGIN_API

/// @ingroup os
/// @{

/// @brief a shared library handle
/// @warning do not access the library handle directly, it is platform specific
typedef struct os_library_t
{
    const char *name;
    os_library_impl_t library;
} os_library_t;

/// @brief a file handle
/// @warning do not access the file handle directly, it is platform specific
typedef struct os_file_t
{
    const char *path;
    os_file_impl_t file;
} os_file_t;

/// shared library api

/// @brief open a shared library from disk
///
/// @param path the path to the library to open
/// @param library the library handle to fill
///
/// @return 0 on success, error otherwise
RET_INSPECT
CT_OS_API os_error_t os_library_open(
        IN_STRING const char *path,
        OUT_NOTNULL os_library_t *library);

/// @brief close a shared library
///
/// @param library the library to close
RET_INSPECT
CT_OS_API os_error_t os_library_close(
        INOUT_NOTNULL os_library_t *library);

/// @brief get a symbol from a shared library
///
/// @param library the library to get the symbol from
/// @param symbol the symbol to fill
/// @param name the name of the symbol to get
///
/// @return the symbol or NULL if it could not be found
CT_NODISCARD
CT_OS_API os_error_t os_library_symbol(
        INOUT_NOTNULL os_library_t *library,
        OUT_NOTNULL os_symbol_t *symbol,
        IN_STRING const char *name);

/// @brief get the name of a shared library
///
/// @param library the library to get the name of
///
/// @return the name of the library
CT_NODISCARD
CT_OS_API const char *os_library_name(IN_NOTNULL const os_library_t *library);

/// filesytem api

/// @brief copy a file from one location to another
///
/// @param src the source file
/// @param dst the destination file
///
/// @return an error if the file could not be copied
RET_INSPECT
CT_OS_API os_error_t os_file_copy(
        IN_STRING const char *dst,
        IN_STRING const char *src);

/// @brief check if a file exists
///
/// @param path the path to the file to check
///
/// @return error if the file could not be checked
/// @retval eOsExists if the file exists
/// @retval eOsNotFound if the file does not exist
RET_INSPECT
CT_OS_API os_error_t os_file_exists(IN_STRING const char *path);

/// @brief create a file
///
/// @param path the path to the file to create
///
/// @return an error if the file could not be created
RET_INSPECT
CT_OS_API os_error_t os_file_create(IN_STRING const char *path);

/// @brief delete a file
///
/// @param path the path to the file to delete
///
/// @return an error if the file could not be deleted
RET_INSPECT
CT_OS_API os_error_t os_file_delete(IN_STRING const char *path);

/// @brief check if a directory exists
///
/// @param path the path to the directory to check
/// @param create true if the directory was created, false if it already existed
///
/// @return an error if the directory could not be checked
RET_INSPECT
CT_OS_API os_error_t os_dir_create(IN_STRING const char *path, OUT_NOTNULL bool *create);

/// @brief delete a directory
///
/// @param path the path to the directory to delete
///
/// @return an error if the directory could not be deleted
RET_INSPECT
CT_OS_API os_error_t os_dir_delete(IN_STRING const char *path);

/// @brief check if a directory exists
///
/// @param path the path to the directory to check
///
/// @return true if the directory exists, false otherwise
RET_INSPECT
CT_OS_API bool os_dir_exists(IN_STRING const char *path);

/// @brief get the type of a paths inode entry
///
/// @param path the path to the inode entry to check
///
/// @return the type of the inode entry
CT_NODISCARD
CT_OS_API os_dirent_t os_dirent_type(IN_STRING const char *path);

/// directory iteration

/// @brief open a directory for iteration
/// @note the iterator must be closed with @a os_iter_end
///
/// @param path path to directory
/// @param iter iterator to fill
///
/// @return result containing either a valid iterator or an error, NULL if dir does not exist
RET_INSPECT
CT_OS_API os_error_t os_iter_begin(IN_STRING const char *path, OUT_NOTNULL os_iter_t *iter);

/// @brief close a directory iterator
///
/// @param iter iterator to close
CT_OS_API os_error_t os_iter_end(OUT_PTR_INVALID os_iter_t *iter);

/// @brief get the next directory entry
///
/// @param iter iterator to use
/// @param dir directory entry to fill
///
/// @return true if a directory entry was found
CT_NODISCARD
CT_OS_API bool os_iter_next(INOUT_NOTNULL os_iter_t *iter, OUT_NOTNULL os_inode_t *dir);

/// @brief get the error state of a directory iterator
///
/// @param iter iterator to check
///
/// @return the error state of the iterator
RET_INSPECT
CT_OS_API os_error_t os_iter_error(IN_NOTNULL const os_iter_t *iter);

/// file api

/// @brief open a file
///
/// @param path the path to the file to open
/// @param access the access mode to open the file with
/// @param file the file handle to fill
///
/// @return an error if the file could not be opened
RET_INSPECT
CT_OS_API os_error_t os_file_open(IN_STRING const char *path, os_access_t access, INOUT_NOTNULL os_file_t *file);

/// @brief create a temporary file
///
/// @param file the file handle to fill
///
/// @return an error if the file could not be created
RET_INSPECT
CT_OS_API os_error_t os_tmpfile_open(OUT_NOTNULL os_file_t *file);

/// @brief close a file
///
/// @param file the file to close
CT_OS_API os_error_t os_file_close(OUT_PTR_INVALID os_file_t *file);

/// @brief read from a file
///
/// @param file the file to read from
/// @param buffer the buffer to read into
/// @param size the number of bytes to read
/// @param actual the number of bytes actually read
///
/// @return the number of bytes read
/// @return an error if the file could not be read from
RET_INSPECT
CT_OS_API os_error_t os_file_read(
        INOUT_NOTNULL os_file_t *file,
        OUT_WRITES(size) void *buffer,
        IN_RANGE(>, 0) size_t size,
        OUT_NOTNULL size_t *actual);

/// @brief write to a file
///
/// @param file the file to write to
/// @param buffer the buffer to write from
/// @param size the number of bytes to write
/// @param actual the number of bytes actually written
///
/// @return the number of bytes written
/// @return an error if the file could not be written to
RET_INSPECT
CT_OS_API os_error_t os_file_write(
        INOUT_NOTNULL os_file_t *file,
        IN_READS(size) const void *buffer,
        IN_RANGE(>, 0) size_t size,
        OUT_NOTNULL size_t *actual);

/// @brief get the size of a file
///
/// @param file the file to get the size of
/// @param actual the size of the file
///
/// @return an error if the file size could not be retrieved
RET_INSPECT
CT_OS_API os_error_t os_file_size(INOUT_NOTNULL os_file_t *file, OUT_NOTNULL size_t *actual);

/// @brief seek to a position in a file
///
/// @param file the file to seek in
/// @param offset the offset to seek to
/// @param actual the actual offset after seeking
///
/// @return an error if the file could not be seeked
RET_INSPECT
CT_OS_API os_error_t os_file_seek(INOUT_NOTNULL os_file_t *file, size_t offset, OUT_NOTNULL size_t *actual);

/// @brief get the current position in a file
///
/// @param file the file to get the position of
/// @param actual the current position in the file
///
/// @return an error if the file position could not be retrieved
RET_INSPECT
CT_OS_API os_error_t os_file_tell(INOUT_NOTNULL os_file_t *file, OUT_NOTNULL size_t *actual);

/// @brief truncate/expand a file to a specific size
///
/// @param file the file to truncate/expand
/// @param size the size to truncate/expand the file to
///
/// @return an error if the operation could not be performed
RET_INSPECT
CT_OS_API os_error_t os_file_expand(INOUT_NOTNULL os_file_t *file, size_t size);

/// @brief map a file into memory
///
/// @param file the file to map
/// @param protect the memory protection to use
/// @param size the size of the mapping, 0 for the whole file
/// @param mapping the mapping to fill
///
/// @return an error if the file could not be mapped
RET_INSPECT
CT_OS_API os_error_t os_file_map(
        INOUT_NOTNULL os_file_t *file,
        os_protect_t protect,
        size_t size,
        OUT_NOTNULL os_mapping_t *mapping);

/// @brief unmap a file from memory
/// @note invalidates all memory pointers returned by @a os_mapping_data
///
/// @param mapping the mapping to unmap
CT_OS_API os_error_t os_unmap(INOUT_NOTNULL os_mapping_t *mapping);

/// @brief get the data of a file mapping
///
/// @param mapping the mapping to get the data of
///
/// @return the data of the mapping
CT_NODISCARD
CT_OS_API void *os_mapping_data(INOUT_NOTNULL os_mapping_t *mapping);

/// @brief get the size of a file mapping
///
/// @param mapping the mapping to get the size of
///
/// @return the size of the mapping
CT_NODISCARD
CT_OS_API size_t os_mapping_size(INOUT_NOTNULL const os_mapping_t *mapping);

/// @brief does the mapping object contain a valid mapping
/// checks if the mapping data exists, not for the validity of the mapping
///
/// @param mapping the mapping to check
///
/// @return true if the mapping is valid
CT_NODISCARD
CT_OS_API bool os_mapping_active(INOUT_NOTNULL const os_mapping_t *mapping);

/// @brief get the name of a file
///
/// @param file the file to get the name of
///
/// @return the name of the file
CT_NODISCARD
CT_OS_API const char *os_file_name(INOUT_NOTNULL const os_file_t *file);

#if CTU_EVENTS

typedef struct os_events_t
{
    void (*on_file_open)(const os_file_t *file);
    void (*on_file_close)(const os_file_t *file);
    void (*on_file_read)(const os_file_t *file, size_t size);
    void (*on_file_write)(const os_file_t *file, size_t size);

    void (*on_library_open)(const os_library_t *library);
    void (*on_library_close)(const os_library_t *library);
    void (*on_library_symbol)(const os_library_t *library, const char *name, const os_symbol_t *symbol);

    void (*on_mapping_open)(const os_file_t *file, const os_mapping_t *mapping);
    void (*on_mapping_close)(const os_mapping_t *mapping);
} os_events_t;

CT_OS_API extern os_events_t gOsEvents;

#endif

/// @}

CT_END_API
