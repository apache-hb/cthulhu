#include "fs2/common.h"

#include "std/vector.h"
#include "std/str.h"
#include "std/map.h"

#include "base/panic.h"

#include "report/report.h"

static vector_t *path_split(const char *path)
{
    return str_split(path, "/");
}

static inode2_t *query_inode(fs2_t *fs, inode2_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode2_is(node, eNodeDir));
    CTASSERT(fs->cb->fnQueryNode != NULL);

    return fs->cb->fnQueryNode(fs, node, name);
}

static inode2_t *create_dir(fs2_t *fs, inode2_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode2_is(node, eNodeDir));
    CTASSERT(fs->cb->fnCreateDir != NULL);

    return fs->cb->fnCreateDir(fs, node, name);
}

static void delete_dir(fs2_t *fs, inode2_t *node, const char *name)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);
    CTASSERT(name != NULL);

    CTASSERT(inode2_is(node, eNodeDir));
    CTASSERT(fs->cb->fnDeleteDir != NULL);

    fs->cb->fnDeleteDir(fs, node, name);
}

static map_t *query_dirents(fs2_t *fs, inode2_t *node)
{
    CTASSERT(fs != NULL);
    CTASSERT(node != NULL);

    CTASSERT(inode2_is(node, eNodeDir));
    CTASSERT(fs->cb->fnQueryDirents != NULL);

    return fs->cb->fnQueryDirents(fs, node);
}

static inode2_t *get_dir_or_create(fs2_t *fs, inode2_t *node, const char *name)
{
    inode2_t *dir = query_inode(fs, node, name);
    switch (dir->type)
    {
    case eNodeDir: return dir;
    case eNodeInvalid: return create_dir(fs, node, name);

    default: return NULL;
    }
}

void fs2_dir_create(fs2_t *fs, const char *path)
{
    vector_t *parts = path_split(path);
    size_t len = vector_len(parts);
    inode2_t *current = fs->root;

    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        inode2_t *node = query_inode(fs, current, part);
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

bool fs2_dir_exists(fs2_t *fs, const char *path)
{
    vector_t *parts = path_split(path);
    size_t len = vector_len(parts);
    inode2_t *current = fs->root;

    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        inode2_t *node = query_inode(fs, current, part);
        switch (node->type)
        {
        case eNodeDir:
            current = node;
            break;

        case eNodeInvalid:
        case eNodeFile:
            return false;

        default:
            CTASSERTF(false, "invalid inode type (type = %d)", node->type);
            return false;
        }
    }

    return inode2_is(current, eNodeDir);
}

void fs2_dir_delete(fs2_t *fs, const char *path)
{
    vector_t *parts = path_split(path);
    size_t len = vector_len(parts);
    inode2_t *current = fs->root;

    CTASSERT(len > 0);

    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        inode2_t *node = query_inode(fs, current, part);
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

static void sync_file(fs2_t *dst, fs2_t *src, inode2_t *dstNode, inode2_t *srcNode)
{
    CTASSERT(inode2_is(dstNode, eNodeFile));
    CTASSERT(inode2_is(srcNode, eNodeFile));

    // TODO: sync file
}

static void sync_dir(fs2_t *dst, fs2_t *src, inode2_t *dstNode, inode2_t *srcNode)
{
    map_t *dirents = query_dirents(src, srcNode);
    map_iter_t iter = map_iter(dirents);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        const char *name = entry.key;
        inode2_t *child = entry.value;

        inode2_t *other = get_dir_or_create(dst, dstNode, name);
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
            break;
        }
    }
}

void fs2_sync(fs2_t *dst, fs2_t *src)
{
    CTASSERT(dst != NULL);
    CTASSERT(src != NULL);

    sync_dir(dst, src, dst->root, src->root);
}
