#include "base/util.h"
#include "common.h"

#include "std/vector.h"
#include "std/str.h"
#include "std/map.h"

#include "io/io.h"

#include "base/panic.h"

static vector_t *path_split(const char *path, arena_t *arena)
{
    return str_split(path, "/", arena);
}

// fs interface api

static inode_t *query_inode(fs_t *fs, inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eOsNodeDir));
    CTASSERT(fs->cb->pfn_query_node != NULL);

    return fs->cb->pfn_query_node(fs, node, name);
}

static map_t *query_dirents(fs_t *fs, inode_t *node)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);

    CTASSERTF(inode_is(node, eOsNodeDir), "invalid inode type (type = %d)", node->type);
    CTASSERT(fs->cb->pfn_query_dirents != NULL);

    return fs->cb->pfn_query_dirents(fs, node);
}

static io_t *query_file(fs_t *fs, inode_t *node, os_access_t flags)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);

    CTASSERT(inode_is(node, eOsNodeFile));
    CTASSERT(fs->cb->pfn_query_file != NULL);

    return fs->cb->pfn_query_file(fs, node, flags);
}

static inode_t *create_dir(fs_t *fs, inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eOsNodeDir));
    CTASSERT(fs->cb->pfn_create_dir != NULL);

    return fs->cb->pfn_create_dir(fs, node, name);
}

static void delete_dir(fs_t *fs, inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eOsNodeDir));
    CTASSERT(fs->cb->pfn_delete_dir != NULL);

    fs->cb->pfn_delete_dir(fs, node, name);
}

static inode_t *create_file(fs_t *fs, inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eOsNodeDir));
    CTASSERT(fs->cb->pfn_create_file != NULL);

    return fs->cb->pfn_create_file(fs, node, name);
}

static void delete_file(fs_t *fs, inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eOsNodeDir));
    CTASSERT(fs->cb->pfn_delete_file != NULL);

    fs->cb->pfn_delete_file(fs, node, name);
}

// fs file api

void fs_file_create(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir: current = node; break;
        default: return;
        }
    }

    create_file(fs, current, vector_tail(parts));
}

bool fs_file_exists(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir: current = node; break;
        default: return false;
        }
    }

    inode_t *file = query_inode(fs, current, vector_tail(parts));
    return inode_is(file, eOsNodeFile);
}

void fs_file_delete(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir: current = node; break;
        default: return;
        }
    }

    delete_file(fs, current, vector_tail(parts));
}

io_t *fs_open(fs_t *fs, const char *path, os_access_t flags)
{
    // create a file if it doesn't exist then open it

    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir: current = node; break;
        case eOsNodeNone: current = create_dir(fs, current, part); break;
        default: return NULL; // TODO: return an invalid file, rather than NULL
        }
    }

    inode_t *file = query_inode(fs, current, vector_tail(parts));
    switch (file->type)
    {
    case eOsNodeFile:
        return query_file(fs, file, flags);
    case eOsNodeNone:
        file = create_file(fs, current, vector_tail(parts));
        return query_file(fs, file, flags);
    default: return NULL; // TODO: same as above
    }
}

// fs dir api

static inode_t *get_dir_or_create(fs_t *fs, inode_t *node, const char *name)
{
    inode_t *dir = query_inode(fs, node, name);
    switch (dir->type)
    {
    case eOsNodeDir: return dir;
    case eOsNodeNone: return create_dir(fs, node, name);

    default: return NULL;
    }
}

static inode_t *get_file_or_create(fs_t *fs, inode_t *node, const char *name)
{
    inode_t *file = query_inode(fs, node, name);
    switch (file->type)
    {
    case eOsNodeFile: return file;
    case eOsNodeNone: return create_file(fs, node, name);

    default: return NULL;
    }
}

static inode_t *get_inode_for(fs_t *fs, inode_t *node, const char *name, os_dirent_t type)
{
    switch (type)
    {
    case eOsNodeDir: return get_dir_or_create(fs, node, name);
    case eOsNodeFile: return get_file_or_create(fs, node, name);
    default: CT_NEVER("invalid inode type for %s", name);
    }
}

void fs_dir_create(fs_t *fs, const char *path)
{
    CTASSERT(fs != NULL);
    CTASSERT(path != NULL);
    CT_PARANOID_ASSERTF(ctu_strlen(path) > 0, "path cannot be empty");

    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir:
            current = node;
            break;
        case eOsNodeNone:
            current = create_dir(fs, current, part);
            break;

        case eOsNodeFile:
            CTASSERTF(false, "cannot create directory with file in path (dir = %s, file = %s)", part, path);
            return;

        default:
            CTASSERTF(false, "invalid inode type (type = %d)", node->type);
            return;
        }
    }
}

bool fs_dir_exists(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
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

void fs_dir_delete(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path, fs->arena);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    CTASSERT(len > 0);

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eOsNodeDir:
            current = node;
            break;

        case eOsNodeNone:
        case eOsNodeFile:
            return;

        default:
            CTASSERTF(false, "invalid inode type (type = %d)", node->type);
            return;
        }
    }

    // TODO: recursively delete all files and directories inside the directory
    delete_dir(fs, current, vector_tail(parts));
}

// fs sync

static void sync_file(fs_t *dst_fs, fs_t *src_fs, inode_t *dst_node, inode_t *src_node)
{
    CTASSERT(inode_is(dst_node, eOsNodeFile));
    CTASSERT(inode_is(src_node, eOsNodeFile));

    io_t *src_io = query_file(src_fs, src_node, eOsAccessRead);
    io_t *dst_io = query_file(dst_fs, dst_node, eOsAccessWrite);

    size_t size = io_size(src_io);
    if (size > 0)
    {
        const void *data = io_map(src_io, eOsProtectRead);
        CTASSERTF(data != NULL, "failed to map file during sync (path = %s)", io_name(src_io));
        io_write(dst_io, data, size);
    }

    io_close(dst_io);
    io_close(src_io);
}

static sync_result_t sync_dir(fs_t *dst, fs_t *src, inode_t *dst_node, inode_t *src_node)
{
    map_t *dirents = query_dirents(src, src_node);
    map_iter_t iter = map_iter(dirents);
    const char *name = NULL;
    inode_t *child = NULL;
    while (CTU_MAP_NEXT(&iter, &name, &child))
    {
        inode_t *other = get_inode_for(dst, dst_node, name, child->type);
        if (other == NULL)
        {
            sync_result_t result = { .path = name };
            return result;
        }

        switch (child->type)
        {
        case eOsNodeDir:
            sync_dir(dst, src, other, child);
            break;
        case eOsNodeFile:
            sync_file(dst, src, other, child);
            break;

        default:
            CT_NEVER("invalid inode type (type = %d)", child->type);
        }
    }

    sync_result_t result = { .path = NULL };
    return result;
}

sync_result_t fs_sync(fs_t *dst, fs_t *src)
{
    CTASSERT(dst != NULL);
    CTASSERT(src != NULL);

    return sync_dir(dst, src, dst->root, src->root);
}

static void iter_dirents(fs_t *fs, inode_t *node, const char *path, const char *name, void *data, fs_dirent_callback_t callback)
{
    CTASSERTF(node != NULL, "invalid inode (fs = %p)", fs);

    callback(path, name, node->type, data);
    if (node->type == eOsNodeFile) return;
    if (node->type != eOsNodeDir) return;

    const char *dir = str_format(fs->arena, "%s/%s", path, name);

    map_t *dirents = query_dirents(fs, node);
    map_iter_t iter = map_iter(dirents);
    const char *id = NULL;
    inode_t *child = NULL;
    while (CTU_MAP_NEXT(&iter, &id, &child))
    {
        iter_dirents(fs, child, dir, id, data, callback);
    }
}

void fs_iter_dirents(fs_t *fs, const char *path, void *data, fs_dirent_callback_t callback)
{
    CTASSERT(fs != NULL);
    CTASSERT(path != NULL);
    CTASSERT(callback != NULL);

    iter_dirents(fs, query_inode(fs, fs->root, path), ".", path, data, callback);
}
