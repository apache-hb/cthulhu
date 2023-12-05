#pragma once

#include "io/io.h"

BEGIN_API

/// @defgroup FS Filesystem abstraction
/// @brief virtual and physical filesystem interface
/// @{

typedef struct logger_t reports_t;
typedef struct fs_t fs_t;

/// @brief create a filesystem interface to a physical location on disk
///
/// @param reports report sink
/// @param root the root directory to mount this filesystem on
///
/// @return a filesystem interface, or NULL if the filesystem failed to mount
fs_t *fs_physical(reports_t *reports, const char *root);

/// @brief create a virtual filesystem interface
///
/// @param reports report sink
/// @param name the name of the vfs
///
/// @return a filesystem interface to an in-memory fs
fs_t *fs_virtual(reports_t *reports, const char *name);

/// @brief create a directory
/// create a directory and all child directories inside a filesystem
///
/// @param fs the filesystem
/// @param path the full path of the directory
void fs_dir_create(fs_t *fs, const char *path);

/// @brief create a file
/// @note this function will not create child directories
///
/// create a file inside a directory
///
/// @param fs the filesystem
/// @param path the full path of the file
void fs_file_create(fs_t *fs, const char *path);

/// @brief open a file at a given location in the filesystem
/// @warning the returned io object is not thread safe
///
/// @param fs the filesystem
/// @param path the path of the file
/// @param flags the access modifiers to use when opening the file
///
/// @return an io object representing the file on disk, or NULL on failure
io_t *fs_open(fs_t *fs, const char *path, os_access_t flags);

/// @brief query if a directory exists
///
/// @param fs the filesystem
/// @param path the path of the directory
///
/// @return true if it exists, false otherwise
bool fs_dir_exists(fs_t *fs, const char *path);

/// @brief query if a file exists
///
/// @param fs the filesystem
/// @param path the path of the file
///
/// @return true if it exists, false otherwise
bool fs_file_exists(fs_t *fs, const char *path);

/// @brief delete a directory
/// delete a directory and all contained files and folders
///
/// @param fs the filesystem
/// @param path the path to the folder
void fs_dir_delete(fs_t *fs, const char *path);

/// @brief delete a file
///
/// @param fs the filesystem
/// @param path the path to the file
void fs_file_delete(fs_t *fs, const char *path);

/// @brief synchronize 2 filesystems
/// copies all folders and files from @arg src to @arg dst
///
/// @param dst the destination filesystem
/// @param src the source filesystem
void fs_sync(fs_t *dst, fs_t *src);

/// @} // FS

END_API
