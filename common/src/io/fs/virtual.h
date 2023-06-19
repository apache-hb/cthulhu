#pragma once

#include "io/io.h"

typedef struct map_t map_t;
typedef struct io_t io_t;

typedef struct vfs_dir_t 
{
    map_t *children; // map_t<const char *, inode_t *>
} vfs_dir_t;

typedef struct vfs_file_t
{
    io_t *io;
} vfs_file_t;

io_t *io_virtual(vfs_file_t *file, const char *name, file_flags_t flags);
