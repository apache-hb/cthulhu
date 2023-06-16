#pragma once

#include "io/fs.h"

typedef struct inode_t inode_t;

/**
 * query for an existing inode in a directory
 */
typedef inode_t *(*inode_query_t)(inode_t *parent, const char *name);

/**
 * create a new directory inode and add it into the parent
 */
typedef inode_t *(*inode_mkdir_t)(inode_t *parent, const char *name);

/**
 * create a new file inode and add it into the parent
 */
typedef inode_t *(*inode_open_t)(inode_t *parent, const char *name, file_flags_t flags);

/**
 * get the io_t for a file inode
 */
typedef io_t *(*inode_file_t)(inode_t *inode);

typedef struct fs_callbacks_t
{
    inode_query_t inodeQuery;
    
    inode_mkdir_t inodeDir;
    inode_open_t inodeOpen;

    inode_file_t inodeFile;
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

    char data[];
} fs_t;



inode_t *inode_dir(inode_t *parent, void *data, size_t size);
inode_t *inode_file(inode_t *parent, void *data, size_t size);

void *inode_data(inode_t *inode);
bool inode_is(inode_t *inode, inode_type_t type);



fs_t *fs_new(const fs_callbacks_t *cb, inode_t *root, void *data, size_t size);

void *fs_data(fs_t *fs);
