#pragma once

#include "io/fs.h"

typedef struct inode_t inode_t;
typedef struct map_t map_t;

/**
 * query for an existing inode in a directory
 */
typedef inode_t *(*fs_query_inode_t)(fs_t *fs, inode_t *parent, const char *name);

/**
 * create a new directory inode and add it into the parent
 */
typedef inode_t *(*fs_create_dir_t)(fs_t *fs, inode_t *parent, const char *name);

/**
 * create a new file inode and add it into the parent
 */
typedef inode_t *(*fs_create_file_t)(fs_t *fs, inode_t *parent, const char *name);

/**
 * delete a directory inode from the parent
 */
typedef void (*fs_delete_dir_t)(fs_t *fs, inode_t *parent, const char *name);

/**
 * delete a file inode from the parent
 */
typedef void (*fs_delete_file_t)(fs_t *fs, inode_t *parent, const char *name);

/**
 * get the io_t for a file inode
 */
typedef io_t *(*fs_get_file_t)(fs_t *fs, inode_t *inode, file_flags_t flags);

/**
 * get all the children of a directory inode
 */
typedef map_t *(*fs_get_dir_t)(fs_t *fs, inode_t *inode);

typedef struct fs_callbacks_t
{
    fs_query_inode_t fnQueryNode;
    
    fs_create_dir_t fnCreateDir;
    fs_create_file_t fnCreateFile;

    fs_delete_dir_t fnDeleteDir;
    fs_delete_file_t fnDeleteFile;

    fs_get_file_t fnGetHandle;
    fs_get_dir_t fnGetDir;
} fs_callbacks_t;

typedef enum inode_type_t
{
    eNodeFile,
    eNodeDir,

    eNodeTotal
} inode_type_t;

typedef struct inode_t 
{
    inode_type_t type;
    
    inode_t *parent;

    char data[];
} inode_t;

typedef struct fs_t 
{
    const fs_callbacks_t *cb;
    inode_t *root;
    const char *path;

    char data[];
} fs_t;

// inode api

inode_t *inode_dir(inode_t *parent, void *data, size_t size);
inode_t *inode_file(inode_t *parent, void *data, size_t size);

void *inode_data(inode_t *inode);
bool inode_is(inode_t *inode, inode_type_t type);

// fs api

fs_t *fs_new(const fs_callbacks_t *cb, const char *path, inode_t *root, void *data, size_t size);

void *fs_data(fs_t *fs);
