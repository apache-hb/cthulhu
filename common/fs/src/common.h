#pragma once

#include "fs/fs.h"

#include <stddef.h>
#include <stdint.h>

typedef struct map_t map_t;

typedef enum inode_type_t
{
    eNodeFile,
    eNodeDir,
    eNodeInvalid,

    eNodeTotal
} inode_type_t;

typedef struct inode_t
{
    inode_type_t type;
    uint32_t shutup; // ubsan moment

    char data[];
} inode_t;

typedef inode_t *(*fs_query_node_t)(fs_t *fs, inode_t *node, const char *name);
typedef map_t *(*fs_query_dirents_t)(fs_t *fs, inode_t *node);
typedef io_t *(*fs_query_file_t)(fs_t *fs, inode_t *node, os_access_t flags);

typedef inode_t *(*fs_dir_create_t)(fs_t *fs, inode_t *node, const char *name);
typedef inode_t *(*fs_file_create_t)(fs_t *fs, inode_t *node, const char *name);

typedef void (*fs_dir_delete_t)(fs_t *fs, inode_t *node, const char *name);
typedef void (*fs_file_delete_t)(fs_t *fs, inode_t *node, const char *name);

/// @brief fs callback to delete the fs
///
/// @param fs the fs to delete
typedef void (*fs_delete_t)(fs_t *fs);

typedef struct fs_callbacks_t
{
    fs_query_node_t pfn_query_node;
    fs_query_dirents_t pfn_query_dirents;
    fs_query_file_t pfn_query_file;

    fs_dir_create_t pfn_create_dir;
    fs_dir_delete_t pfn_delete_dir;

    fs_file_create_t pfn_create_file;
    fs_file_delete_t pfn_delete_file;

    /// @brief delete the fs
    /// optional callback if the fs needs extra cleanup.
    /// eg, releasing locks or other resources
    fs_delete_t pfn_delete;
} fs_callbacks_t;

typedef struct fs_t
{
    const fs_callbacks_t *cb; ///< callbacks
    inode_t *root; ///< root inode

    char data[];
} fs_t;

// inode api

extern inode_t gInvalidINode;

inode_t *inode_file(const void *data, size_t size);
inode_t *inode_dir(const void *data, size_t size);
void *inode_data(inode_t *inode);
bool inode_is(inode_t *inode, inode_type_t type);

// helpers

OS_RESULT(bool) mkdir_recursive(const char *path);

// fs api

fs_t *fs_new(inode_t *root, const fs_callbacks_t *cb, const void *data, size_t size);

void *fs_data(fs_t *fs);
