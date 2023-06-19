#pragma once

#include "io/fs.h"
#include <stddef.h>

typedef enum inode2_type_t
{
    eNodeFile,
    eNodeDir,

    eNodeTotal
} inode2_type_t;

typedef struct inode2_t
{
    inode2_type_t type;

    char data[];
} inode2_t;

typedef struct fs2_interface_t
{
    const char *name;
} fs2_interface_t;

typedef struct fs2_t 
{
    const fs2_interface_t *cb; ///< callbacks
    reports_t *reports; ///< reports
    inode2_t *root; ///< root inode

    char data[];
} fs2_t;

// inode api

inode2_t *inode2_file(const void *data, size_t size);
inode2_t *inode2_dir(const void *data, size_t size);
void *inode2_data(inode2_t *inode);

// fs api

fs2_t *fs2_new(reports_t *reports, inode2_t *root, const fs2_interface_t *cb, const void *data, size_t size);

void *fs2_data(fs2_t *fs);
