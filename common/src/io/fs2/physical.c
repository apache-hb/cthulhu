#include "common.h"

#include "std/str.h"
#include "std/map.h"

typedef struct physical_t
{
    const char *root; ///< absolute path to root directory
} physical_t;

typedef struct physical_file_t
{
    const char *path; ///< path to file relative to root
} physical_file_t;

typedef struct physical_dir_t
{
    const char *path; ///< path to directory relative to root
} physical_dir_t;

static char *get_absolute(fs2_t *fs, const char *path)
{
    const physical_t *self = fs2_data(fs);

    return format("%s" NATIVE_PATH_SEPARATOR "%s", self->root, path);
}

static inode2_t *physical_dir(const char *path)
{
    physical_dir_t dir = {
        .path = path
    };
    
    return inode2_dir(&dir, sizeof(physical_dir_t));
}

static inode2_t *physical_file(const char *path)
{
    physical_file_t file = {
        .path = path
    };

    return inode2_file(&file, sizeof(physical_file_t));
}

static inode2_t *pfs_query_node(fs2_t *fs, inode2_t *self, const char *name)
{
    char *absolute = get_absolute(fs, name);
    if (dir_exists(absolute))
    {
        return physical_dir(absolute);
    }
    
    if (file_exists(absolute))
    {
        return physical_file(absolute);
    }
    
    return NULL;
}

static io_t *pfs_query_file(fs2_t *fs, inode2_t *self, file_flags_t flags)
{
    physical_file_t *file = inode2_data(self);
    char *absolute = get_absolute(fs, file->path);
    return io_file(absolute, flags);
}

static inode2_t *pfs_file_create(fs2_t *fs, inode2_t *self, const char *name)
{
    char *absolute = get_absolute(fs, name);
    if (!file_exists(absolute))
    {
        // TODO: a little hacky
        cerror_t err = 0;
        file_t fd = file_open(absolute, eFileWrite, &err);
        file_close(fd);
    }
    
    return physical_file(absolute);
}

static inode2_t *pfs_dir_create(fs2_t *fs, inode2_t *self, const char *name)
{
    char *absolute = get_absolute(fs, name);
    if (!dir_exists(absolute))
    {
        dir_create(absolute);
    }
    
    return physical_dir(absolute);
}

static void pfs_dir_delete(fs2_t *fs, inode2_t *self, const char *name)
{
    char *absolute = get_absolute(fs, name);
    if (dir_exists(absolute))
    {
        dir_delete(absolute);
    }
}

static void pfs_file_delete(fs2_t *fs, inode2_t *self, const char *name)
{
    char *absolute = get_absolute(fs, name);
    if (file_exists(absolute))
    {
        file_delete(absolute);
    }
}

static const fs2_interface_t kPhysicalInterface = {
    .name = "Disk File System",

    .fnQueryNode = pfs_query_node,
    .fnQueryDirents = NULL, // TODO: query dirents
    .fnQueryFile = pfs_query_file,

    .fnCreateDir = pfs_dir_create,
    .fnDeleteDir = pfs_dir_delete,

    .fnCreateFile = pfs_file_create,
    .fnDeleteFile = pfs_file_delete
};

fs2_t *fs2_physical(reports_t *reports, const char *root)
{
    physical_t self = {
        .root = root
    };

    inode2_t *dir = physical_dir(".");

    return fs2_new(reports, dir, &kPhysicalInterface, &self, sizeof(physical_t));
}
