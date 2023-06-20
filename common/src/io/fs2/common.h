#pragma once

#include "io/fs.h"
#include <stddef.h>

typedef struct map_t map_t;

typedef enum inode2_type_t
{
    eNodeFile,
    eNodeDir,
    eNodeInvalid,

    eNodeTotal
} inode2_type_t;

typedef struct inode2_t
{
    inode2_type_t type;

    char data[];
} inode2_t;

typedef inode2_t *(*fs2_query_node_t)(fs2_t *fs, inode2_t *node, const char *name);
typedef map_t *(*fs2_query_dirents_t)(fs2_t *fs, inode2_t *node);

typedef inode2_t *(*fs2_dir_create_t)(fs2_t *fs, inode2_t *node, const char *name);
typedef inode2_t *(*fs2_file_create_t)(fs2_t *fs, inode2_t *node, const char *name, file_flags_t flags);

typedef void (*fs2_dir_delete_t)(fs2_t *fs, inode2_t *node, const char *name);
typedef void (*fs2_file_delete_t)(fs2_t *fs, inode2_t *node, const char *name);

typedef struct fs2_interface_t
{
    const char *name;

    fs2_query_node_t fnQueryNode;
    fs2_query_dirents_t fnQueryDirents;

    fs2_dir_create_t fnCreateDir;
    fs2_dir_delete_t fnDeleteDir;
} fs2_interface_t;

typedef struct fs2_t 
{
    const fs2_interface_t *cb; ///< callbacks
    reports_t *reports; ///< reports
    inode2_t *root; ///< root inode

    char data[];
} fs2_t;

// inode api

extern inode2_t kInvalidINode;

inode2_t *inode2_file(const void *data, size_t size);
inode2_t *inode2_dir(const void *data, size_t size);
void *inode2_data(inode2_t *inode);
bool inode2_is(inode2_t *inode, inode2_type_t type);

// fs api

fs2_t *fs2_new(reports_t *reports, inode2_t *root, const fs2_interface_t *cb, const void *data, size_t size);

void *fs2_data(fs2_t *fs);
