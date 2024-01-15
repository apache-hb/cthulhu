#pragma once

#include <ctu_fs_api.h>

#include "os/os.h"

BEGIN_API

typedef struct arena_t arena_t;
typedef struct io_t io_t;

/// @defgroup filesystem Filesystem abstraction
/// @brief virtual and physical filesystem interface
/// @ingroup common
/// @{

/// @brief a filesystem interface
typedef struct fs_t fs_t;

/// @brief create a filesystem interface to a physical location on disk
///
/// @param root the root directory to mount this filesystem on
/// @param arena the arena to allocate from
///
/// @return a filesystem interface, or NULL if the filesystem failed to mount
NODISCARD
CT_FS_API fs_t *fs_physical(const char *root, arena_t *arena);

/// @brief create a virtual filesystem interface
///
/// @param name the name of the vfs
/// @param arena the arena to allocate from
///
/// @return a filesystem interface to an in-memory fs
NODISCARD
CT_FS_API fs_t *fs_virtual(const char *name, arena_t *arena);

/// @brief create a directory
/// create a directory and all child directories inside a filesystem
///
/// @param fs the filesystem
/// @param path the full path of the directory
CT_FS_API void fs_dir_create(fs_t *fs, const char *path);

/// @brief create a file
/// @note this function will not create child directories
///
/// create a file inside a directory
///
/// @param fs the filesystem
/// @param path the full path of the file
CT_FS_API void fs_file_create(fs_t *fs, const char *path);

/// @brief open a file at a given location in the filesystem
/// @warning the returned io object is not thread safe
///
/// @param fs the filesystem
/// @param path the path of the file
/// @param flags the access modifiers to use when opening the file
///
/// @return an io object representing the file on disk, or NULL on failure
NODISCARD
CT_FS_API io_t *fs_open(fs_t *fs, const char *path, os_access_t flags);

/// @brief query if a directory exists
///
/// @param fs the filesystem
/// @param path the path of the directory
///
/// @return true if it exists, false otherwise
NODISCARD
CT_FS_API bool fs_dir_exists(fs_t *fs, const char *path);

/// @brief query if a file exists
///
/// @param fs the filesystem
/// @param path the path of the file
///
/// @return true if it exists, false otherwise
NODISCARD
CT_FS_API bool fs_file_exists(fs_t *fs, const char *path);

/// @brief delete a directory
/// delete a directory and all contained files and folders
///
/// @param fs the filesystem
/// @param path the path to the folder
CT_FS_API void fs_dir_delete(fs_t *fs, const char *path);

/// @brief delete a file
///
/// @param fs the filesystem
/// @param path the path to the file
CT_FS_API void fs_file_delete(fs_t *fs, const char *path);

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

/// @}

END_API
