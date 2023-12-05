#include "common.h"

#include "std/vector.h"
#include "std/str.h"
#include "std/map.h"

#include "base/panic.h"
#include "memory/memory.h"

#include "report/report.h"

#include <string.h>

static vector_t *path_split(const char *path)
{
    return str_split(path, "/");
}

// fs interface api

static inode_t *query_inode(fs_t *fs, inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eNodeDir));
    CTASSERT(fs->cb->pfn_query_node != NULL);

    return fs->cb->pfn_query_node(fs, node, name);
}

static map_t *query_dirents(fs_t *fs, inode_t *node)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);

    CTASSERT(inode_is(node, eNodeDir));
    CTASSERT(fs->cb->pfn_query_dirents != NULL);

    return fs->cb->pfn_query_dirents(fs, node);
}

static io_t *query_file(fs_t *fs, inode_t *node, os_access_t flags)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);

    CTASSERT(inode_is(node, eNodeFile));
    CTASSERT(fs->cb->pfn_query_file != NULL);

    return fs->cb->pfn_query_file(fs, node, flags);
}

static inode_t *create_dir(fs_t *fs, inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eNodeDir));
    CTASSERT(fs->cb->pfn_create_dir != NULL);

    return fs->cb->pfn_create_dir(fs, node, name);
}

static void delete_dir(fs_t *fs, inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eNodeDir));
    CTASSERT(fs->cb->pfn_delete_dir != NULL);

    fs->cb->pfn_delete_dir(fs, node, name);
}

static inode_t *create_file(fs_t *fs, inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eNodeDir));
    CTASSERT(fs->cb->pfn_create_file != NULL);

    return fs->cb->pfn_create_file(fs, node, name);
}

static void delete_file(fs_t *fs, inode_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode_is(node, eNodeDir));
    CTASSERT(fs->cb->pfn_delete_file != NULL);

    fs->cb->pfn_delete_file(fs, node, name);
}

// fs file api

void fs_file_create(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eNodeDir: current = node; break;
        default: return;
        }
    }

    create_file(fs, current, vector_tail(parts));
}

bool fs_file_exists(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eNodeDir: current = node; break;
        default: return false;
        }
    }

    inode_t *file = query_inode(fs, current, vector_tail(parts));
    return inode_is(file, eNodeFile);
}

void fs_file_delete(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eNodeDir: current = node; break;
        default: return;
        }
    }

    delete_file(fs, current, vector_tail(parts));
}

io_t *fs_open(fs_t *fs, const char *path, os_access_t flags)
{
    // create a file if it doesn't exist then open it

    vector_t *parts = path_split(path);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eNodeDir: current = node; break;
        case eNodeInvalid: current = create_dir(fs, current, part); break;
        default: return NULL;
        }
    }

    inode_t *file = query_inode(fs, current, vector_tail(parts));
    switch (file->type)
    {
    case eNodeFile:
        return query_file(fs, file, flags);
    case eNodeInvalid:
        file = create_file(fs, current, vector_tail(parts));
        return query_file(fs, file, flags);
    default: return NULL;
    }
}

// fs dir api

static inode_t *get_dir_or_create(fs_t *fs, inode_t *node, const char *name)
{
    inode_t *dir = query_inode(fs, node, name);
    switch (dir->type)
    {
    case eNodeDir: return dir;
    case eNodeInvalid: return create_dir(fs, node, name);

    default: return NULL;
    }
}

static inode_t *get_file_or_create(fs_t *fs, inode_t *node, const char *name)
{
    inode_t *file = query_inode(fs, node, name);
    switch (file->type)
    {
    case eNodeFile: return file;
    case eNodeInvalid: return create_file(fs, node, name);

    default: return NULL;
    }
}

static inode_t *get_inode_for(fs_t *fs, inode_t *node, const char *name, inode_type_t type)
{
    switch (type)
    {
    case eNodeDir: return get_dir_or_create(fs, node, name);
    case eNodeFile: return get_file_or_create(fs, node, name);
    default: NEVER("invalid inode type for %s", name);
    }
}

void fs_dir_create(fs_t *fs, const char *path)
{
    CTASSERT(fs != NULL);
    CTASSERT(path != NULL);
    CTASSERT(strlen(path) > 0);

    vector_t *parts = path_split(path);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eNodeDir:
            current = node;
            break;
        case eNodeInvalid:
            current = create_dir(fs, current, part);
            break;

        case eNodeFile:
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
    vector_t *parts = path_split(path);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eNodeDir:
            current = node;
            break;

        case eNodeInvalid:
        case eNodeFile:
            return false;

        default:
            NEVER("invalid inode type (type = %d)", node->type);
        }
    }

    return inode_is(current, eNodeDir);
}

void fs_dir_delete(fs_t *fs, const char *path)
{
    vector_t *parts = path_split(path);
    size_t len = vector_len(parts);
    inode_t *current = fs->root;

    CTASSERT(len > 0);

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eNodeDir:
            current = node;
            break;

        case eNodeInvalid:
        case eNodeFile:
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

static void sync_file(fs_t *dst, fs_t *src, inode_t *dstNode, inode_t *srcNode)
{
    CTASSERT(inode_is(dstNode, eNodeFile));
    CTASSERT(inode_is(srcNode, eNodeFile));

    io_t *srcIo = query_file(src, srcNode, eAccessRead);
    io_t *dstIo = query_file(dst, dstNode, eAccessWrite);

    size_t size = io_size(srcIo);
    if (size > 0)
    {
        void *data = ctu_malloc(size);

        size_t read = io_read(srcIo, data, size);
        io_write(dstIo, data, read);
    }

    io_close(srcIo);
    io_close(dstIo);
}

static void sync_dir(fs_t *dst, fs_t *src, inode_t *dstNode, inode_t *srcNode)
{
    map_t *dirents = query_dirents(src, srcNode);
    map_iter_t iter = map_iter(dirents);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        const char *name = entry.key;
        inode_t *child = entry.value;

        inode_t *other = get_inode_for(dst, dstNode, name, child->type);
        if (other == NULL)
        {
            report(src->reports, eWarn, NULL, "cannot create directory (path = %s)", name);
            return;
        }

        switch (child->type)
        {
        case eNodeDir:
            sync_dir(dst, src, other, child);
            break;
        case eNodeFile:
            sync_file(dst, src, other, child);
            break;

        default:
            NEVER("invalid inode type (type = %d)", child->type);
        }
    }
}

void fs_sync(fs_t *dst, fs_t *src)
{
    CTASSERT(dst != NULL);
    CTASSERT(src != NULL);

    sync_dir(dst, src, dst->root, src->root);
}