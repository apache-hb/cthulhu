// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_fs_api.h>

#include "os/core.h"

#include <stdbool.h>

CT_BEGIN_API

typedef struct arena_t arena_t;
typedef struct io_t io_t;
typedef struct vector_t vector_t;

/// @defgroup filesystem Filesystem abstraction
/// @brief virtual and physical filesystem interface
/// @ingroup common
/// @{

typedef struct fs_t fs_t;
typedef struct fs_inode_t fs_inode_t;
typedef struct fs_iter_t fs_iter_t;

/// @brief delete a filesystem handle
///
/// @param fs the filesystem to delete
CT_FS_API void fs_delete(OUT_PTR_INVALID fs_t *fs);

/// @brief create a filesystem interface to a physical location on disk
///
/// @param root the root directory to mount this filesystem on
/// @param arena the arena to allocate from
///
/// @return a filesystem interface, or NULL if the filesystem failed to mount
CT_NODISCARD
CT_FS_API fs_t *fs_physical(IN_STRING const char *root, IN_NOTNULL arena_t *arena);

/// @brief create a virtual filesystem interface
///
/// @param name the name of the vfs
/// @param arena the arena to allocate from
///
/// @return a filesystem interface to an in-memory fs
CT_NODISCARD
CT_FS_API fs_t *fs_virtual(IN_STRING const char *name, IN_NOTNULL arena_t *arena);

/// @brief create a directory
/// create a directory and all child directories inside a filesystem
///
/// @param fs the filesystem
/// @param path the full path of the directory
///
/// @return 0 on success, an error code otherwise
CT_FS_API os_error_t fs_dir_create(IN_NOTNULL fs_t *fs, IN_STRING const char *path);

/// @brief create a file
/// @note this function will not create intermediate directories
///
/// create a file inside a directory
///
/// @param fs the filesystem
/// @param path the full path of the file
CT_FS_API void fs_file_create(IN_NOTNULL fs_t *fs, IN_STRING const char *path);

/// @brief open a file at a given location in the filesystem
/// @warning the returned io object is not thread safe
///
/// @param fs the filesystem
/// @param path the path of the file
/// @param flags the access modifiers to use when opening the file
///
/// @return an io object representing the file on disk
CT_NODISCARD
CT_FS_API io_t *fs_open(IN_NOTNULL fs_t *fs, IN_STRING const char *path, os_access_t flags);

/// @brief query if a directory exists
///
/// @param fs the filesystem
/// @param path the path of the directory
///
/// @return true if it exists, false otherwise
CT_NODISCARD
CT_FS_API bool fs_dir_exists(IN_NOTNULL fs_t *fs, IN_STRING const char *path);

/// @brief query if a file exists
///
/// @param fs the filesystem
/// @param path the path of the file
///
/// @return true if it exists, false otherwise
CT_NODISCARD
CT_FS_API bool fs_file_exists(IN_NOTNULL fs_t *fs, IN_STRING const char *path);

/// @brief delete a directory
/// delete a directory and all contained files and folders
///
/// @param fs the filesystem
/// @param path the path to the folder
///
/// @return 0 on success, an error code otherwise
CT_FS_API os_error_t fs_dir_delete(IN_NOTNULL fs_t *fs, IN_STRING const char *path);

/// @brief delete a file
///
/// @param fs the filesystem
/// @param path the path to the file
///
/// @return 0 on success, an error code otherwise
CT_FS_API os_error_t fs_file_delete(IN_NOTNULL fs_t *fs, IN_STRING const char *path);

/// @brief the result of a @a fs_sync call
/// this is here because we cant use @ref notify in the fs api
typedef struct sync_result_t
{
    /// @brief the file that failed to sync
    /// @note this will be NULL if the sync succeeded
    const char *path;
} sync_result_t;

/// @brief synchronize 2 filesystems
/// copies all folders and files from @p src to @p dst
///
/// @param dst the destination filesystem
/// @param src the source filesystem
CT_FS_API sync_result_t fs_sync(fs_t *dst, fs_t *src);

/// @brief get the type of an inode
///
/// @param inode the inode to get the type of
///
/// @return the type of the inode
CT_PUREFN
CT_FS_API os_dirent_t fs_inode_type(IN_NOTNULL const fs_inode_t *inode);

/// @brief check if a given inode is of a certain type
///
/// @param inode the inode to check
/// @param type the type to check against
///
/// @return true if the inode is of the given type, false otherwise
CT_PUREFN
CT_FS_API bool fs_inode_is(IN_NOTNULL const fs_inode_t *inode, os_dirent_t type);

/// @brief get the name of an inode
/// @note the name is only valid for the lifetime of the inode
///
/// @param inode the inode to get the name of
///
/// @return the name of the inode
CT_PUREFN
CT_FS_API const char *fs_inode_name(IN_NOTNULL const fs_inode_t *inode);

/// @brief find an inode in a filesystem
///
/// @param fs the filesystem to search
/// @param path the path to the inode
///
/// @return the inode if it exists, NULL otherwise
CT_PUREFN
CT_FS_API fs_inode_t *fs_find_inode(IN_NOTNULL fs_t *fs, IN_STRING const char *path);

/// @brief get the root inode of a filesystem
///
/// @param fs the filesystem to get the root inode of
///
/// @return the root inode of the filesystem
CT_PUREFN
CT_FS_API fs_inode_t *fs_root_inode(IN_NOTNULL fs_t *fs);

/// @brief begin iterating over a directory
///
/// @param fs the filesystem to iterate
/// @param path the path to the directory
/// @param iter the iterator to create
///
/// @return eOsSuccess on success, an error code otherwise
CT_FS_API os_error_t fs_iter_begin(
    IN_NOTNULL fs_t *fs,
    IN_STRING const fs_inode_t *path,
    OUT_NOTNULL fs_iter_t **iter);

/// @brief end an iteration
///
/// @param iter the iterator to end
///
/// @return eOsSuccess on success, an error code otherwise
CT_FS_API os_error_t fs_iter_end(
    IN_NOTNULL fs_iter_t *iter);

/// @brief get the next inode in an iteration
///
/// @param iter the iterator to get the next inode from
/// @param inode the inode to write to
///
/// @return eOsSuccess on success, an error code otherwise
/// @retval eOsNotFound if there are no more inodes
CT_FS_API os_error_t fs_iter_next(
    IN_NOTNULL fs_iter_t *iter,
    OUT_NOTNULL fs_inode_t **inode);

/// @}

CT_END_API
