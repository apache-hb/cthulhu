// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "fs/fs.h"

#include <stddef.h>

typedef struct map_t map_t;
typedef struct arena_t arena_t;

typedef struct inode_result_t
{
    fs_inode_t *node;
    os_error_t error;
} inode_result_t;

typedef fs_inode_t *(*fs_query_node_t)(fs_t *fs, fs_inode_t *node, const char *name);
typedef map_t *(*fs_query_dirents_t)(fs_t *fs, fs_inode_t *node);
typedef io_t *(*fs_query_file_t)(fs_t *fs, fs_inode_t *node, os_access_t flags);

typedef inode_result_t (*fs_file_create_t)(fs_t *fs, fs_inode_t *node, const char *name);
typedef os_error_t (*fs_file_delete_t)(fs_t *fs, fs_inode_t *node, const char *name);

typedef inode_result_t (*fs_dir_create_t)(fs_t *fs, fs_inode_t *node, const char *name);
typedef os_error_t (*fs_dir_delete_t)(fs_t *fs, fs_inode_t *node, const char *name);

/// @brief fs callback to delete the fs
///
/// @param fs the fs to delete
typedef void (*fs_delete_t)(fs_t *fs);

typedef struct fs_callbacks_t
{
    fs_query_node_t pfn_query_node;

    // TODO: return an iterator instead of doing it in serial
    fs_query_dirents_t pfn_query_dirents;
    fs_query_file_t pfn_query_file;

    fs_dir_create_t pfn_create_dir;
    fs_dir_delete_t pfn_delete_dir;

    fs_file_create_t pfn_create_file;
    fs_file_delete_t pfn_delete_file;
} fs_callbacks_t;

typedef struct fs_t
{
    const fs_callbacks_t *cb; ///< callbacks
    arena_t *arena;
    fs_inode_t *root; ///< root inode

    char data[];
} fs_t;

// inode api

CT_LOCAL extern fs_inode_t gInvalidFileNode;

CT_LOCAL fs_inode_t *inode_file(const void *data, size_t size, arena_t *arena);
CT_LOCAL fs_inode_t *inode_dir(const void *data, size_t size, arena_t *arena);

CT_LOCAL void *inode_data(fs_inode_t *inode);
CT_LOCAL bool inode_is(fs_inode_t *inode, os_dirent_t type);

// helpers
CT_LOCAL os_error_t mkdir_recursive(const char *path, bool *success, arena_t *arena);

// fs api

CT_LOCAL fs_t *fs_new(fs_inode_t *root, const fs_callbacks_t *cb, const void *data, size_t size, arena_t *arena);

CT_LOCAL void *fs_data(fs_t *fs);
