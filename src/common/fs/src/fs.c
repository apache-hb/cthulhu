// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

#include "arena/arena.h"

#include "os/os.h"

#include "io/impl.h"
#include "io/io.h"

#include "std/vector.h"
#include "std/str.h"
#include "std/map.h"

#include "base/util.h"
#include "base/panic.h"

static vector_t *path_split(const char *path, arena_t *arena)
{
    return str_split(path, "/", arena);
}

// fs interface api

static fs_inode_t *impl_query_inode(fs_t *fs, fs_inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eOsNodeDir));
    CTASSERT(fs->cb->pfn_query_node != NULL);

    return fs->cb->pfn_query_node(fs, node, name);
}

static io_t *impl_query_file(fs_t *fs, fs_inode_t *node, os_access_t flags)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);

    CTASSERT(inode_is(node, eOsNodeFile));
    CTASSERT(fs->cb->pfn_query_file != NULL);

    return fs->cb->pfn_query_file(fs, node, flags);
}

static fs_inode_t *impl_create_file(fs_t *fs, fs_inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eOsNodeDir));
    CTASSERT(fs->cb->pfn_create_file != NULL);

    inode_result_t result = fs->cb->pfn_create_file(fs, node, name);

    return result.node;
}

static os_error_t impl_delete_file(fs_t *fs, fs_inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eOsNodeDir));
    CTASSERT(fs->cb->pfn_delete_file != NULL);

    return fs->cb->pfn_delete_file(fs, node, name);
}

static fs_inode_t *impl_create_dir(fs_t *fs, fs_inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eOsNodeDir));
    CTASSERT(fs->cb->pfn_create_dir != NULL);

    inode_result_t result = fs->cb->pfn_create_dir(fs, node, name);

    return result.node;
}

static os_error_t impl_delete_dir(fs_t *fs, fs_inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eOsNodeDir));
    CTASSERT(fs->cb->pfn_delete_dir != NULL);

    return fs->cb->pfn_delete_dir(fs, node, name);
}

static os_error_t impl_iter_begin(fs_t *fs, const fs_inode_t *dir, fs_iter_t *iter)
{
    CTASSERT(fs != NULL);
    CTASSERT(dir != NULL);
    CTASSERT(iter != NULL);

    CTASSERT(fs_inode_is(dir, eOsNodeDir));
    CTASSERT(fs->cb->pfn_iter_begin != NULL);

    return fs->cb->pfn_iter_begin(fs, dir, iter);
}

static os_error_t impl_iter_next(fs_iter_t *iter)
{
    CTASSERT(iter != NULL);
    CTASSERT(iter->fs != NULL);

    CTASSERT(iter->fs->cb->pfn_iter_next != NULL);

    return iter->fs->cb->pfn_iter_next(iter);
}

static os_error_t impl_iter_end(fs_iter_t *iter)
{
    CTASSERT(iter != NULL);
    CTASSERT(iter->fs != NULL);

    CTASSERT(iter->fs->cb->pfn_iter_end != NULL);

    return iter->fs->cb->pfn_iter_end(iter);
}

// private impl

static fs_inode_t *find_inode(fs_t *fs, fs_inode_t *start, const char *path)
{
    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    fs_inode_t *current = start;

    CTASSERT(len > 0);

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        fs_inode_t *node = impl_query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir:
            current = node;
            break;

        case eOsNodeNone:
        case eOsNodeFile:
            return NULL;

        default:
            CT_NEVER("invalid inode type (%s)", os_dirent_string(node->type));
        }
    }

    return impl_query_inode(fs, current, vector_tail(parts));
}

// fs delete

STA_DECL
void fs_delete(fs_t *fs)
{
    CTASSERT(fs != NULL);

    arena_free(fs, CT_ALLOC_SIZE_UNKNOWN, fs->arena);
}

// fs file api

STA_DECL
void fs_file_create(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    fs_inode_t *current = fs->root;

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        fs_inode_t *node = impl_query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir: current = node; break;
        default: return;
        }
    }

    impl_create_file(fs, current, vector_tail(parts));
}

STA_DECL
bool fs_file_exists(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    fs_inode_t *current = fs->root;

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        fs_inode_t *node = impl_query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir: current = node; break;
        default: return false;
        }
    }

    fs_inode_t *file = impl_query_inode(fs, current, vector_tail(parts));
    return inode_is(file, eOsNodeFile);
}

STA_DECL
os_error_t fs_file_delete(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    fs_inode_t *current = fs->root;

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        fs_inode_t *node = impl_query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir: current = node; break;
        default: return eOsNotFound;
        }
    }

    return impl_delete_file(fs, current, vector_tail(parts));
}

static const io_callbacks_t kInvalidIo = { 0 };

static io_t *make_invalid_file(const char *name, os_access_t flags, arena_t *arena)
{
    io_t *io = io_new(&kInvalidIo, flags, name, NULL, arena);
    io->error = eOsNotFound;
    return io;
}

STA_DECL
io_t *fs_open(fs_t *fs, const char *path, os_access_t flags)
{
    // create a file if it doesn't exist then open it

    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    fs_inode_t *current = fs->root;

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        fs_inode_t *node = impl_query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir: current = node; break;
        case eOsNodeNone: current = impl_create_dir(fs, current, part); break;
        default: return make_invalid_file(path, flags, fs->arena);
        }
    }

    fs_inode_t *file = impl_query_inode(fs, current, vector_tail(parts));
    switch (file->type)
    {
    case eOsNodeFile:
        return impl_query_file(fs, file, flags);
        break;
    case eOsNodeNone:
        if (flags == eOsAccessRead)
            return make_invalid_file(path, eOsAccessNone, fs->arena);

        file = impl_create_file(fs, current, vector_tail(parts));
        return impl_query_file(fs, file, flags);
        break;
    default:
        return make_invalid_file(path, flags, fs->arena);
    }
}

// fs dir api

static fs_inode_t *get_dir_or_create(fs_t *fs, fs_inode_t *node, const char *name)
{
    fs_inode_t *dir = impl_query_inode(fs, node, name);
    switch (dir->type)
    {
    case eOsNodeDir: return dir;
    case eOsNodeNone: return impl_create_dir(fs, node, name);

    default: return NULL;
    }
}

static fs_inode_t *get_file_or_create(fs_t *fs, fs_inode_t *node, const char *name)
{
    fs_inode_t *file = impl_query_inode(fs, node, name);
    switch (file->type)
    {
    case eOsNodeFile: return file;
    case eOsNodeNone: return impl_create_file(fs, node, name);

    default: return NULL;
    }
}

static fs_inode_t *get_inode_for(fs_t *fs, fs_inode_t *node, const char *name, os_dirent_t type)
{
    switch (type)
    {
    case eOsNodeDir: return get_dir_or_create(fs, node, name);
    case eOsNodeFile: return get_file_or_create(fs, node, name);
    default: CT_NEVER("invalid inode type for %s", name);
    }
}

STA_DECL
os_error_t fs_dir_create(fs_t *fs, const char *path)
{
    CTASSERT(fs != NULL);
    CTASSERT(path != NULL);
    CT_PARANOID_ASSERTF(ctu_strlen(path) > 0, "path cannot be empty");

    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    fs_inode_t *current = fs->root;

    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        fs_inode_t *node = impl_query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir:
            current = node;
            break;
        case eOsNodeNone:
            current = impl_create_dir(fs, current, part);
            break;

        case eOsNodeFile:
            return eOsExists;

        default:
            CT_NEVER("invalid inode type (type = %d)", node->type);
        }
    }

    return eOsSuccess;
}

STA_DECL
bool fs_dir_exists(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    fs_inode_t *current = fs->root;

    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        fs_inode_t *node = impl_query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir:
            current = node;
            break;

        case eOsNodeNone:
        case eOsNodeFile:
            return false;

        default:
            CT_NEVER("invalid inode type (type = %d)", node->type);
        }
    }

    return inode_is(current, eOsNodeDir);
}

STA_DECL
os_error_t fs_dir_delete(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    fs_inode_t *current = fs->root;

    CTASSERT(len > 0);

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        fs_inode_t *node = impl_query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir:
            current = node;
            break;

        case eOsNodeNone:
        case eOsNodeFile:
            return eOsNotFound;

        default:
            CT_NEVER("invalid inode type (type = %d)", node->type);
        }
    }

    // TODO: recursively delete all files and directories inside the directory
    return impl_delete_dir(fs, current, vector_tail(parts));
}

// fs sync

static os_error_t sync_file(fs_t *dst_fs, fs_t *src_fs, fs_inode_t *dst_node, fs_inode_t *src_node)
{
    CTASSERT(inode_is(dst_node, eOsNodeFile));
    CTASSERT(inode_is(src_node, eOsNodeFile));

    os_error_t err = eOsSuccess;
    io_t *src_io = impl_query_file(src_fs, src_node, eOsAccessRead);
    io_t *dst_io = impl_query_file(dst_fs, dst_node, eOsAccessWrite | eOsAccessTruncate);

    if ((err = io_error(src_io)) != eOsSuccess)
    {
        goto cleanup;
    }

    if ((err = io_error(dst_io)) != eOsSuccess)
    {
        goto cleanup;
    }

    size_t size = io_size(src_io);
    if (size > 0)
    {
        const void *data = io_map(src_io, eOsProtectRead);
        CTASSERTF(data != NULL, "failed to map file during sync (path = %s)", io_name(src_io));
        io_write(dst_io, data, size);
    }

cleanup:
    // TODO: do we care about the error from closing the io?
    io_close(dst_io);
    io_close(src_io);
    return err;
}

static sync_result_t sync_dir(fs_t *dst, fs_t *src, fs_inode_t *dst_node, fs_inode_t *src_node)
{
    fs_iter_t *iter;
    os_error_t err = eOsSuccess;

    err = fs_iter_begin(src, src_node, &iter);
    if (err != eOsSuccess)
    {
        sync_result_t result = { .path = NULL };
        return result;
    }

    fs_inode_t *child;
    while (fs_iter_next(iter, &child) == eOsSuccess)
    {
        fs_inode_t *other = get_inode_for(dst, dst_node, child->name, child->type);
        if (other == NULL)
        {
            sync_result_t result = { .path = child->name };
            return result;
        }

        if (child->type == eOsNodeDir)
        {
            sync_dir(dst, src, other, child);
        }
        else if (child->type == eOsNodeFile)
        {
            if (sync_file(dst, src, other, child) != eOsSuccess)
            {
                sync_result_t result = { .path = child->name };
                return result;
            }
        }
        else
        {
            CT_NEVER("invalid inode type (type = %d)", child->type);
        }
    }

    fs_iter_end(iter);

    sync_result_t result = { .path = NULL };
    return result;
}

STA_DECL
sync_result_t fs_sync(fs_t *dst, fs_t *src)
{
    CTASSERT(dst != NULL);
    CTASSERT(src != NULL);

    return sync_dir(dst, src, dst->root, src->root);
}

STA_DECL
os_dirent_t fs_inode_type(const fs_inode_t *inode)
{
    CTASSERT(inode != NULL);

    return inode->type;
}

STA_DECL
bool fs_inode_is(const fs_inode_t *inode, os_dirent_t type)
{
    CT_ASSERT_RANGE(type, 0, eOsNodeCount - 1);

    return fs_inode_type(inode) == type;
}

STA_DECL
const char *fs_inode_name(const fs_inode_t *inode)
{
    CTASSERT(inode != NULL);

    return inode->name;
}

STA_DECL
fs_inode_t *fs_find_inode(fs_t *fs, const char *path)
{
    CTASSERT(fs != NULL);
    CTASSERT(path != NULL);

    return find_inode(fs, fs->root, path);
}

STA_DECL
fs_inode_t *fs_root_inode(fs_t *fs)
{
    CTASSERT(fs != NULL);

    return fs->root;
}

STA_DECL
os_error_t fs_iter_begin(fs_t *fs, const fs_inode_t *node, fs_iter_t **iter)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(iter != NULL);

    if (!fs_inode_is(node, eOsNodeDir))
    {
        return eOsExists;
    }
    const size_t sz = sizeof(fs_iter_t) + fs->cb->iter_size;

    fs_iter_t *data = ARENA_MALLOC(sz, "fs_iter", node, fs->arena);
    data->fs = fs;
    data->dir = node;
    data->current = NULL;

    os_error_t err = impl_iter_begin(fs, node, data);
    if (err != eOsSuccess)
    {
        arena_free(data, sz, fs->arena);
        return err;
    }

    *iter = data;
    return eOsSuccess;
}

STA_DECL
os_error_t fs_iter_end(fs_iter_t *iter)
{
    CTASSERT(iter != NULL);
    const size_t sz = sizeof(fs_iter_t) + iter->fs->cb->iter_size;

    os_error_t err = impl_iter_end(iter);
    arena_free(iter, sz, iter->fs->arena);

    return err;
}

STA_DECL
os_error_t fs_iter_next(fs_iter_t *iter, fs_inode_t **inode)
{
    CTASSERT(iter != NULL);
    CTASSERT(inode != NULL);

    os_error_t err = impl_iter_next(iter);
    if (err == eOsSuccess)
    {
        *inode = iter->current;
    }

    return err;
}
