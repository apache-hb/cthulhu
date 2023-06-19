#include "common.h"

#include "platform/error.h"

#include "std/str.h"

#include "base/panic.h"

#include "report/report.h"

typedef struct pfs_t
{
    const char *absolute; ///< absolute path
} pfs_t;

typedef struct pfs_dir_t
{
    const char *relative; ///< relative to parent or root
} pfs_dir_t;

typedef struct pfs_file_t
{
    const char *relative; ///< relative to parent or root
} pfs_file_t;

static inode_t *phys_dir_new(inode_t *parent, const char *relative)
{
    pfs_dir_t dir = {
        .relative = relative
    };

    return inode_dir(parent, &dir, sizeof(pfs_dir_t));
}

static inode_t *phys_file_new(inode_t *parent, const char *relative)
{
    pfs_file_t file = {
        .relative = relative
    };

    return inode_file(parent, &file, sizeof(pfs_file_t));
}

static const char *get_absolute_path(fs_t *fs, inode_t *node, const char *name)
{
    pfs_t *pfs = fs_data(fs);
    if (node == NULL)
    {
        return format("%s" NATIVE_PATH_SEPARATOR "%s", pfs->absolute, name);
    }

    pfs_dir_t *dir = inode_data(node);
    char *relative = format("%s" NATIVE_PATH_SEPARATOR "%s", dir->relative, name);

    return get_absolute_path(fs, node->parent, relative);
}

static inode_t *phys_query(fs_t *fs, inode_t *inode, const char *name)
{
    const char *absolute = get_absolute_path(fs, inode, name);

    if (dir_exists(absolute))
    {
        return phys_dir_new(inode, name); 
    }

    if (file_exists(absolute))
    { 
        return phys_file_new(inode, name); 
    }

    return NULL;
}

static inode_t *phys_create_dir(fs_t *fs, inode_t *parent, const char *name)
{
    const char *absolute = get_absolute_path(fs, parent, name);

    dir_create(absolute);

    return phys_dir_new(parent, name);
}

static inode_t *phys_create_file(fs_t *fs, inode_t *parent, const char *name)
{
    // TODO: thonk
    return phys_file_new(parent, name);
}

static void phys_delete_dir(fs_t *fs, inode_t *inode, const char *path)
{
    const char *absolute = get_absolute_path(fs, inode, path);

    dir_delete(absolute);
}

static void phys_delete_file(fs_t *fs, inode_t *inode, const char *path)
{
    const char *absolute = get_absolute_path(fs, inode, path);

    file_delete(absolute);
}

static io_t *phys_get_handle(fs_t *fs, inode_t *inode, const char *name, file_flags_t flags)
{
    UNUSED(name);
    const char *absolute = get_absolute_path(fs, inode, NULL);

    io_t *io = io_file(absolute, flags);
    CTASSERT(io != NULL);

    return io;
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
    const char *cwd = get_cwd();

    const char *absolute = format("%s" NATIVE_PATH_SEPARATOR "%s", cwd, root);
    dir_create(absolute);

    logverbose("dir-create (cwd = %s, abs = %s)", cwd, absolute);

    pfs_t fs = { .absolute = cwd };

    inode_t *node = phys_dir_new(NULL, root);

    return fs_new(&kPhysical, node, &fs, sizeof(pfs_t));
}
