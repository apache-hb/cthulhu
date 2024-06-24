// SPDX-License-Identifier: LGPL-3.0-only

#include "arena/arena.h"
#include "base/util.h"
#include "common.h"

#include "std/str.h"
#include "std/map.h"

#include "os/os.h"
#include "io/impl.h"

#include "base/panic.h"
#include <stdint.h>

typedef struct physical_t
{
    const char *root; ///< absolute path to root directory
} physical_t;

typedef struct physical_inode_t
{
    const char *path; ///< path to file or directory relative to root
} physical_inode_t;

static const char *get_absolute(fs_t *fs, const fs_inode_t *node, const char *path)
{
    CTASSERT(fs != NULL);

    const physical_t *self = fs_data(fs);
    const physical_inode_t *dir = inode_data((fs_inode_t*)node);

    if (is_path_special(dir->path) && is_path_special(path))
    {
        return self->root;
    }

    if (is_path_special(dir->path) && !is_path_special(path))
    {
        return str_format(fs->arena, "%s" CT_NATIVE_PATH_SEPARATOR "%s", self->root, path);
    }

    if (!is_path_special(dir->path) && is_path_special(path))
    {
        return str_format(fs->arena, "%s" CT_NATIVE_PATH_SEPARATOR "%s", self->root, dir->path);
    }

    return str_format(fs->arena, "%s" CT_NATIVE_PATH_SEPARATOR "%s" CT_NATIVE_PATH_SEPARATOR "%s", self->root, dir->path, path);
}

static const char *get_relative(const fs_inode_t *node, const char *path, arena_t *arena)
{
    const physical_inode_t *dir = inode_data((fs_inode_t*)node);

    if (is_path_special(dir->path) && !is_path_special(path))
    {
        return path;
    }

    if (!is_path_special(dir->path) && is_path_special(path))
    {
        return dir->path;
    }

    CTASSERT(!is_path_special(dir->path) && !is_path_special(path));

    return str_format(arena, "%s" CT_NATIVE_PATH_SEPARATOR "%s", dir->path, path);
}

static fs_inode_t *physical_dir(fs_t *fs, const char *path)
{
    char *id = arena_strdup(path, fs->arena);
    physical_inode_t inode = {
        .path = id
    };

    const char *name = id;

    size_t i = str_rfind(id, CT_NATIVE_PATH_SEPARATOR);
    if (i != SIZE_MAX)
    {
        name = id + i + 1;
    }

    return inode_dir(fs, name, &inode);
}

static fs_inode_t *physical_file(fs_t *fs, const char *path)
{
    char *id = arena_strdup(path, fs->arena);
    physical_inode_t inode = {
        .path = id
    };

    const char *name = id;

    size_t i = str_rfind(id, CT_NATIVE_PATH_SEPARATOR);
    if (i != SIZE_MAX)
    {
        name = id + i + 1;
    }

    return inode_file(fs, name, &inode);
}

static fs_inode_t *pfs_query_node(fs_t *fs, const fs_inode_t *self, const char *name)
{
    const char *absolute = get_absolute(fs, self, name);
    os_dirent_t dirent = os_dirent_type(absolute);
    CTASSERTF(dirent != eOsNodeError, "failed to query node %s", absolute);

    const char *relative = get_relative(self, name, fs->arena);

    switch (dirent)
    {
    case eOsNodeFile:
        return physical_file(fs, relative);
    case eOsNodeDir:
        return physical_dir(fs, relative);
    default:
        return &gInvalidFileNode;
    }
}

static io_t *pfs_query_file(fs_t *fs, fs_inode_t *self, os_access_t flags)
{
    const char *absolute = get_absolute(fs, self, NULL);
    return io_file(absolute, flags, fs->arena);
}

static inode_result_t pfs_file_create(fs_t *fs, fs_inode_t *self, const char *name)
{
    const char *absolute = get_absolute(fs, self, name);
    os_error_t err = os_file_create(absolute);
    if (err != eOsSuccess)
    {
        inode_result_t result = { .error = err };
        return result;
    }

    fs_inode_t *inode = physical_file(fs, get_relative(self, name, fs->arena));
    inode_result_t result = { .node = inode };
    return result;
}

static inode_result_t pfs_dir_create(fs_t *fs, fs_inode_t *self, const char *name)
{
    const char *absolute = get_absolute(fs, self, name);
    os_error_t err = mkdir_recursive(absolute, fs->arena);
    if (err != eOsSuccess && err != eOsExists)
    {
        inode_result_t result = { .error = err };
        return result;
    }

    fs_inode_t *inode = physical_dir(fs, get_relative(self, name, fs->arena));
    inode_result_t result = { .node = inode };
    return result;
}

static os_error_t pfs_dir_delete(fs_t *fs, fs_inode_t *self, const char *name)
{
    const char *absolute = get_absolute(fs, self, name);
    return os_dir_delete(absolute);
}

static os_error_t pfs_file_delete(fs_t *fs, fs_inode_t *self, const char *name)
{
    const char *absolute = get_absolute(fs, self, name);
    return os_file_delete(absolute);
}

static os_error_t pfs_iter_begin(fs_t *fs, const fs_inode_t *dir, fs_iter_t *iter)
{
    const char *absolute = get_absolute(fs, dir, NULL);
    os_iter_t *data = iter_data(iter);
    return os_iter_begin(absolute, data);
}

static os_error_t pfs_iter_next(fs_iter_t *iter)
{
    os_inode_t cur = { 0 };
    if (!os_iter_next(iter_data(iter), &cur))
        return eOsNotFound;

    iter->current = pfs_query_node(iter->fs, iter->dir, os_inode_name(&cur));

    return eOsSuccess;
}

static os_error_t pfs_iter_end(fs_iter_t *iter)
{
    return os_iter_end(iter_data(iter));
}

static const fs_callbacks_t kPhysicalInterface = {
    .pfn_query_node = pfs_query_node,
    .pfn_query_file = pfs_query_file,

    .pfn_create_dir = pfs_dir_create,
    .pfn_delete_dir = pfs_dir_delete,

    .pfn_create_file = pfs_file_create,
    .pfn_delete_file = pfs_file_delete,

    .pfn_iter_begin = pfs_iter_begin,
    .pfn_iter_next = pfs_iter_next,
    .pfn_iter_end = pfs_iter_end,

    .iter_size = sizeof(os_iter_t),
    .inode_size = sizeof(physical_inode_t),
};

STA_DECL
fs_t *fs_physical(const char *root, arena_t *arena)
{
    CTASSERT(root != NULL);

    if (!os_dir_exists(root))
    {
        os_error_t err = mkdir_recursive(root, arena);

        if (err != eOsSuccess && err != eOsExists)
        {
            return NULL;
        }
    }

    physical_t self = {
        .root = root
    };

    physical_inode_t inode = {
        .path = "."
    };

    return fs_new(&inode, &kPhysicalInterface, &self, sizeof(physical_t), arena);
}
