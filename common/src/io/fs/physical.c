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

static char *get_absolute(fs_t *fs, const char *path)
{
    const physical_t *self = fs_data(fs);

    return format("%s" NATIVE_PATH_SEPARATOR "%s", self->root, path);
}

static inode_t *physical_dir(const char *path)
{
    physical_dir_t dir = {
        .path = path
    };
    
    return inode_dir(&dir, sizeof(physical_dir_t));
}

static inode_t *physical_file(const char *path)
{
    physical_file_t file = {
        .path = path
    };

    return inode_file(&file, sizeof(physical_file_t));
}

static inode_t *pfs_query_node(fs_t *fs, inode_t *self, const char *name)
{
    char *absolute = get_absolute(fs, name);
    OS_RESULT(os_dirent_t) dirent = os_dirent_type(absolute);
    switch (OS_VALUE_OR(os_dirent_t, dirent, eOsNodeNone))
    {
    case eOsNodeFile: return physical_file(absolute);
    case eOsNodeDir: return physical_dir(absolute);
    default: return NULL;
    }
}

static map_t *pfs_query_dirents(fs_t *fs, inode_t *self)
{
    physical_dir_t *dir = inode_data(self);
    char *absolute = get_absolute(fs, dir->path);

    OS_RESULT(os_iter_t*) iter = os_iter_begin(absolute);
    OS_RESULT(os_dir_t*) node = NULL;

    if (os_error(iter)) { return map_new(1); }

    os_iter_t *it = OS_VALUE(os_iter_t*, iter);

    map_t *dirents = map_new(64);

    while ((node = os_iter_next(it)) != NULL)
    {
        if (os_error(node)) { break; }
        os_dir_t *dir = OS_VALUE(os_dir_t*, node);
        const char *path = os_dir_name(dir);

        inode_t *inode = pfs_query_node(fs, self, path);
        map_set(dirents, path, inode);
    }

    os_iter_end(it);

    return dirents;
}

static io_t *pfs_query_file(fs_t *fs, inode_t *self, os_access_t flags)
{
    physical_file_t *file = inode_data(self);
    char *absolute = get_absolute(fs, file->path);
    return io_file(absolute, flags);
}

static inode_t *pfs_file_create(fs_t *fs, inode_t *self, const char *name)
{
    char *absolute = get_absolute(fs, name);
    OS_RESULT(bool) check = os_file_create(absolute);
    return physical_file(absolute);
}

static inode_t *pfs_dir_create(fs_t *fs, inode_t *self, const char *name)
{
    char *absolute = get_absolute(fs, name);
    OS_RESULT(bool) check = os_dir_create(absolute);

    return physical_dir(absolute);
}

static void pfs_dir_delete(fs_t *fs, inode_t *self, const char *name)
{
    char *absolute = get_absolute(fs, name);
    OS_RESULT(bool) check = os_dir_delete(absolute);
}

static void pfs_file_delete(fs_t *fs, inode_t *self, const char *name)
{
    char *absolute = get_absolute(fs, name);
    OS_RESULT(bool) check = os_file_delete(absolute);
}

static const fs_interface_t kPhysicalInterface = {
    .fnQueryNode = pfs_query_node,
    .fnQueryDirents = pfs_query_dirents,
    .fnQueryFile = pfs_query_file,

    .fnCreateDir = pfs_dir_create,
    .fnDeleteDir = pfs_dir_delete,

    .fnCreateFile = pfs_file_create,
    .fnDeleteFile = pfs_file_delete
};

fs_t *fs_physical(reports_t *reports, const char *root)
{
    physical_t self = {
        .root = root
    };

    inode_t *dir = physical_dir(".");

    return fs_new(reports, dir, &kPhysicalInterface, &self, sizeof(physical_t));
}
