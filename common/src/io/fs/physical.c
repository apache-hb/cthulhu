#include "common.h"

#include "std/str.h"

#include "base/panic.h"

#include "report/report.h"

typedef struct pfs_t
{
    const char *root; ///< absolute path
} pfs_t;

typedef struct pfs_dir_t
{
    const char *path; ///< relative to root
} pfs_dir_t;

typedef struct pfs_file_t
{
    const char *path;
} pfs_file_t;

static inode_t *phys_dir_new(inode_t *parent, const char *path)
{
    pfs_dir_t dir = {
        .path = path
    };

    return inode_dir(parent, &dir, sizeof(pfs_dir_t));
}

static inode_t *phys_file_new(inode_t *parent, const char *path)
{
    pfs_file_t file = {
        .path = path
    };

    return inode_file(parent, &file, sizeof(pfs_file_t));
}

static inode_t *phys_query(fs_t *fs, inode_t *inode, const char *name)
{
    pfs_t *pfs = fs_data(fs);
    pfs_dir_t *dir = inode_data(inode);

    char *path = format("%s/%s/%s", pfs->root, dir->path, name);
    if (is_directory(path))
    {
        return phys_dir_new(inode, path);
    }
    
    if (is_file(path))
    {
        return phys_file_new(inode, path);
    }

    return NULL;
}

static inode_t *phys_create_dir(fs_t *fs, inode_t *parent, const char *name)
{
    pfs_t *pfs = fs_data(fs);
    pfs_dir_t *dir = inode_data(parent);

    char *path = format("%s/%s/%s", pfs->root, name);
    cerror_t err = make_directory(path);
    if (err != 0)
    {
        logverbose("failed to create directory '%s' (%s)", path, error_string(err));
        return NULL;
    }

    return phys_dir_new(parent, path);
}

static inode_t *phys_create_file(fs_t *fs, inode_t *parent, const char *name)
{
    pfs_t *pfs = fs_data(fs);
    pfs_dir_t *dir = inode_data(parent);

    char *path = format("%s/%s/%s", pfs->root, dir->path, name);
    return phys_file_new(parent, path);
}

static void phys_delete_dir(fs_t *fs, inode_t *inode, const char *path)
{
    pfs_t *pfs = fs_data(fs);
    pfs_dir_t *dir = inode_data(inode);

    char *fp = format("%s/%s/%s", pfs->root, dir->path, path);

    delete_directory(fp);
}

static void phys_delete_file(fs_t *fs, inode_t *inode, const char *path)
{
    pfs_t *pfs = fs_data(fs);
    pfs_file_t *file = inode_data(inode);

    char *fp = format("%s/%s/%s", pfs->root, file->path, path);
    delete_file(fp);
}

static io_t *phys_get_handle(fs_t *fs, inode_t *inode, file_flags_t flags)
{
    UNUSED(fs);
    
    pfs_file_t *file = inode_data(inode);

    return io_file(file->path, flags);
}

static const fs_callbacks_t kPhysical = {
    .fnQueryNode = phys_query,

    .fnCreateDir = phys_create_dir,
    .fnCreateFile = phys_create_file,

    .fnDeleteDir = phys_delete_dir,
    .fnDeleteFile = phys_delete_file,

    .fnGetHandle = phys_get_handle
};

fs_t *fs_physical(const char *root)
{
    pfs_t fs = {
        .root = root
    };

    inode_t *node = phys_dir_new(NULL, root);

    return fs_new(&kPhysical, root, node, &fs, sizeof(pfs_t));
}
